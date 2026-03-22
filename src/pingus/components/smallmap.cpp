// src/pingus/components/smallmap.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2000 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/components/smallmap.hpp"

#include <climits>

#include "pingus/collision_map.hpp"
#include "pingus/components/playfield.hpp"
#include "pingus/groundtype.hpp"
#include "pingus/pingu.hpp"
#include "pingus/pingu_holder.hpp"
#include "pingus/server.hpp"
#include "pingus/smallmap_image.hpp"
#include "pingus/world.hpp"

namespace pingus {

SmallMap::SmallMap(Server* server_, Playfield* playfield_, const Rect& rect_) :
  RectComponent(rect_),
  server(server_),
  playfield(playfield_),
  exit_sur(),
  entrance_sur(),
  image(),
  scroll_mode(),
  has_focus(),
  gc_ptr(nullptr),
  m_composite(rect_.get_width(), rect_.get_height()),
  m_composite_sprite(),
  m_last_view_left(INT_MIN),
  m_last_view_top(INT_MIN),
  m_last_colmap_serial(UINT_MAX)
{
  image = std::unique_ptr<SmallMapImage>(new SmallMapImage(server, rect.get_width(), rect.get_height()));
  scroll_mode = false;
}

SmallMap::~SmallMap()
{
}

void
SmallMap::rebuild_composite(const Rect& view_rect)
{
  CollisionMap* colmap = server->get_world()->get_colmap();

  const int cmap_w = colmap->get_width();
  const int cmap_h = colmap->get_height();
  const int w      = m_composite.get_width();
  const int h      = m_composite.get_height();
  const int pitch  = m_composite.get_pitch();

  m_composite.lock();
  uint8_t* d = m_composite.get_data();

  // Regenerate terrain pixels (same logic as SmallMapImage::update_surface).
  // Byte order 0=R 1=G 2=B 3=A matches SmallMapImage on all platforms.
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      int i = y * pitch + 4 * x;
      switch (colmap->getpixel_fast(x * cmap_w / w, y * cmap_h / h))
      {
        case Groundtype::GP_NOTHING:
          d[i]=0;   d[i+1]=0;   d[i+2]=0;   d[i+3]=255; break;
        case Groundtype::GP_BRIDGE:
          d[i]=0;   d[i+1]=255; d[i+2]=100; d[i+3]=255; break;
        case Groundtype::GP_SOLID:
          d[i]=100; d[i+1]=100; d[i+2]=125; d[i+3]=255; break;
        case Groundtype::GP_WATER:
        case Groundtype::GP_LAVA:
          d[i]=0;   d[i+1]=0;   d[i+2]=200; d[i+3]=255; break;
        default:
          d[i]=200; d[i+1]=200; d[i+2]=200; d[i+3]=255; break;
      }
    }
  }

  // Bake the camera box border directly into the composite surface.
  // The border is in surface-local coordinates (origin = minimap top-left).
  const int bx1 = view_rect.left   - rect.left;
  const int by1 = view_rect.top    - rect.top;
  const int bx2 = view_rect.right  - rect.left;
  const int by2 = view_rect.bottom - rect.top;

  // Top edge
  if (by1 >= 0 && by1 < h)
    for (int x = std::max(bx1,0); x < std::min(bx2,w); ++x)
    { int i=by1*pitch+4*x; d[i]=0; d[i+1]=255; d[i+2]=0; d[i+3]=255; }

  // Bottom edge
  const int by_b = by2 - 1;
  if (by_b >= 0 && by_b < h && by_b != by1)
    for (int x = std::max(bx1,0); x < std::min(bx2,w); ++x)
    { int i=by_b*pitch+4*x; d[i]=0; d[i+1]=255; d[i+2]=0; d[i+3]=255; }

  // Left edge (skip corners)
  if (bx1 >= 0 && bx1 < w)
    for (int y = std::max(by1+1,0); y < std::min(by_b,h); ++y)
    { int i=y*pitch+4*bx1; d[i]=0; d[i+1]=255; d[i+2]=0; d[i+3]=255; }

  // Right edge (skip corners)
  const int bx_r = bx2 - 1;
  if (bx_r >= 0 && bx_r < w && bx_r != bx1)
    for (int y = std::max(by1+1,0); y < std::min(by_b,h); ++y)
    { int i=y*pitch+4*bx_r; d[i]=0; d[i+1]=255; d[i+2]=0; d[i+3]=255; }

  m_composite.unlock();

  m_composite_sprite    = Sprite(m_composite.clone());
  m_last_view_left      = view_rect.left;
  m_last_view_top       = view_rect.top;
  m_last_colmap_serial  = colmap->get_serial();
}

void
SmallMap::draw(DrawingContext& gc)
{
  gc_ptr = &gc;

  World* const& world = server->get_world();

  Vector2i of = playfield->get_pos();
  Rect view_rect;

  if (world->get_width() > gc.get_width())
  {
    int rwidth = int(gc.get_width()  * rect.get_width()  / world->get_width());
    view_rect.left  = rect.left + (of.x * rect.get_width()  / world->get_width()) - rwidth/2;
    view_rect.right = view_rect.left + rwidth;
  }
  else
  {
    view_rect.left  = rect.left;
    view_rect.right = rect.left + rect.get_width();
  }

  if (world->get_height() > gc.get_height())
  {
    int rheight = int(gc.get_height() * rect.get_height() / world->get_height());
    view_rect.top    = rect.top + (of.y * rect.get_height() / world->get_height()) - rheight/2;
    view_rect.bottom = view_rect.top + rheight;
  }
  else
  {
    view_rect.top    = rect.top;
    view_rect.bottom = rect.top + rect.get_height();
  }

  // Rebuild the composite (terrain + camera box) when camera moves or
  // terrain changes.  The composite is blitted as one full-minimap texture,
  // so the 1-pixel green border scales as part of the larger surface rather
  // than being blitted independently.  Individual 1-pixel blits at fractional
  // logical scale (0.8x at 640x480/800x600) can resolve to 0 physical pixels
  // and vanish; embedding them in a larger blit avoids this entirely.
  const unsigned int serial = world->get_colmap()->get_serial();
  if (!m_composite_sprite               ||
      view_rect.left != m_last_view_left ||
      view_rect.top  != m_last_view_top  ||
      serial         != m_last_colmap_serial)
  {
    rebuild_composite(view_rect);
  }

  gc.draw(m_composite_sprite, Vector2i(rect.left, rect.top));

  server->get_world()->draw_smallmap(this);

  // Draw Pingus
  PinguHolder* pingus = world->get_pingus();
  for(PinguIter i = pingus->begin(); i != pingus->end(); ++i)
  {
    int x = static_cast<int>(static_cast<float>(rect.left) + ((*i)->get_x() * static_cast<float>(rect.get_width())
                                                              / static_cast<float>(world->get_width())));
    int y = static_cast<int>(static_cast<float>(rect.top)  + ((*i)->get_y() * static_cast<float>(rect.get_height())
                                                              / static_cast<float>(world->get_height())));

    gc.draw_line(Vector2i(x, y), Vector2i(x, y-2), Color(255, 255, 0));
  }

  gc_ptr = nullptr;
}

void
SmallMap::update(float delta)
{
  image->update(delta);
}

void
SmallMap::draw_sprite(Sprite sprite, Vector3f pos)
{
  World* world = server->get_world();
  float x = static_cast<float>(rect.left) + (pos.x * static_cast<float>(rect.get_width())  / static_cast<float>(world->get_width()));
  float y = static_cast<float>(rect.top)  + (pos.y * static_cast<float>(rect.get_height()) / static_cast<float>(world->get_height()));

  gc_ptr->draw(sprite, Vector3f(x, y));
}

bool
SmallMap::is_at(int x, int y)
{
  return (x > rect.left && x < rect.left + static_cast<int>(rect.get_width())
          && y > rect.top && y < rect.top + static_cast<int>(rect.get_height()));
}

void
SmallMap::on_pointer_move(int x, int y)
{
  int cx, cy;
  World* world = server->get_world();

  if (scroll_mode)
  {
    cx = (x - rect.left) * static_cast<int>(world->get_width()  / rect.get_width());
    cy = (y - rect.top)  * static_cast<int>(world->get_height() / rect.get_height());
    playfield->set_viewpoint(cx, cy);
  }
}

void
SmallMap::on_primary_button_press(int x, int y)
{
  scroll_mode = true;

  int cx, cy;
  World* world = server->get_world();
  cx = (x - rect.left) * int(world->get_width())  / rect.get_width();
  cy = (y - rect.top)  * int(world->get_height()) / rect.get_height();
  playfield->set_viewpoint(cx, cy);
}

void
SmallMap::on_primary_button_release(int /*x*/, int /*y*/)
{
  scroll_mode = false;
}

void
SmallMap::on_pointer_enter()
{
  has_focus = true;
}

void
SmallMap::on_pointer_leave()
{
  has_focus = false;
}


} // namespace pingus

// EOF
