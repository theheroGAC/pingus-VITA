// src/pingus/worldobjs/surface_background.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1999 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/worldobjs/surface_background.hpp"

#include "engine/display/scene_context.hpp"
#include "pingus/globals.hpp"
#include "pingus/resource.hpp"
#include "pingus/world.hpp"

namespace pingus::worldobjs {

SurfaceBackground::SurfaceBackground(const FileReader& reader) :
  para_x(0.5),
  para_y(0.5),
  pos(),
  scroll_x(0.0),
  scroll_y(0.0),
  color(0,0,0,0),
  stretch_x(false),
  stretch_y(false),
  keep_aspect(false),
  bg_sprite(),
  scroll_ox(0),
  scroll_oy(0)
{
  if (!reader.read_vector("position", pos))
    pos = Vector3f(0.f, 0.f, -150.f);

  ResDescriptor desc;

  reader.read_desc("surface", desc);
  if (!reader.read_colori("colori", color))
  {
    Colorf tmp_colorf;
    if (reader.read_colorf("color", tmp_colorf))
    {
      color = tmp_colorf.to_color();
    }
  }

  reader.read_float("para-x", para_x);
  reader.read_float("para-y", para_y);

  reader.read_float("scroll-x", scroll_x);
  reader.read_float("scroll-y", scroll_y);

  reader.read_bool("stretch-x", stretch_x);
  reader.read_bool("stretch-y", stretch_y);

  reader.read_bool("keep-aspect", keep_aspect);

  if (!stretch_x && !stretch_y && color.a == 0)
  {
    // FIXME: would be nice to allow surface manipulation with
    // animated sprites, but it's not that easy to do
    bg_sprite = Sprite(desc);
  }
  else
  {
    Surface surface = Resource::load_surface(desc);

    if (color.a != 0 && surface.is_indexed())
    {
      if (surface.has_colorkey())
      {
        surface = surface.convert_to_rgba();
      }
      else
      {
        surface = surface.convert_to_rgb();
      }
    }

    surface.fill(color);

    // Scaling Code
    if (stretch_x && stretch_y)
    {
      surface = surface.scale(world->get_width(), world->get_height());
    }
    else if (stretch_x && !stretch_y)
    {
      if (keep_aspect)
      {
        float aspect = static_cast<float>(surface.get_height()) / static_cast<float>(surface.get_width());
        surface = surface.scale(world->get_width(), static_cast<int>(static_cast<float>(world->get_width()) * aspect));
      }
      else
      {
        surface = surface.scale(world->get_width(), surface.get_height());
      }
    }
    else if (!stretch_x && stretch_y)
    {
      if (keep_aspect)
      {
        float aspect = static_cast<float>(surface.get_width()) / static_cast<float>(surface.get_height());
        surface = surface.scale(static_cast<int>(static_cast<float>(world->get_height()) * aspect), world->get_height());
      }
      else
      {
        surface = surface.scale(surface.get_width(), world->get_height());
      }
    }

    bg_sprite = Sprite(surface);
  }
}

float
SurfaceBackground::get_z_pos () const
{
  return pos.z;
}

void
SurfaceBackground::update()
{
  bg_sprite.update();

  if (!bg_sprite)
    return;

  if (scroll_x)
  {
    scroll_ox += scroll_x;

    if (scroll_ox > bg_sprite.get_width())
      scroll_ox -= static_cast<float>(bg_sprite.get_width());
    else if (-scroll_ox > bg_sprite.get_width())
      scroll_ox += static_cast<float>(bg_sprite.get_width());
  }

  if (scroll_y)
  {
    scroll_oy += scroll_y;

    if (scroll_oy > bg_sprite.get_height())
      scroll_oy -= static_cast<float>(bg_sprite.get_height());
    else if (-scroll_oy > bg_sprite.get_height())
      scroll_oy += static_cast<float>(bg_sprite.get_height());
  }
}

void
SurfaceBackground::draw (SceneContext& gc)
{
  if (!bg_sprite)
    return;

  Vector2i offset = gc.color().world_to_screen(Vector2i(0,0));

  offset.x -= gc.color().get_rect().left;
  offset.y -= gc.color().get_rect().top;

  int start_x = static_cast<int>((static_cast<float>(offset.x) * para_x) + scroll_ox);
  int start_y = static_cast<int>((static_cast<float>(offset.y) * para_y) + scroll_oy);

  if (start_x > 0)
    start_x = (start_x % bg_sprite.get_width()) - bg_sprite.get_width();

  if (start_y > 0)
    start_y = (start_y % bg_sprite.get_height()) - bg_sprite.get_height();

  // Ensure the first tile's left screen edge is at or before x=0 (and
  // equivalently for y).  When para_x < 1 the background scrolls slower than
  // the camera: start_x = offset.x * para_x, and for a rightward-scrolled
  // camera offset.x is negative, so start_x > offset.x.  The first tile's
  // screen position (start_x - offset.x) is therefore positive, leaving an
  // uncovered strip along the left edge of the viewport that widens as the
  // camera moves further right.  The loop below pulls start_x back by one
  // tile width at a time until the tile is guaranteed to cover screen x=0.
  while ((start_x - offset.x) > 0)
    start_x -= bg_sprite.get_width();

  while ((start_y - offset.y) > 0)
    start_y -= bg_sprite.get_height();

  for(int y = start_y;
      y < world->get_height();
      y += bg_sprite.get_height())
  {
    for(int x = start_x;
        x < world->get_width();
        x += bg_sprite.get_width())
    {
      gc.color().draw(bg_sprite, Vector2i(x - offset.x, y - offset.y), pos.z);
    }
  }
}

} // namespace pingus::worldobjs

// EOF
