// src/pingus/components/smallmap.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2000 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_PINGUS_COMPONENTS_SMALLMAP_HPP
#define HEADER_PINGUS_PINGUS_COMPONENTS_SMALLMAP_HPP

#include "engine/display/sprite.hpp"
#include "engine/display/surface.hpp"
#include "engine/gui/rect_component.hpp"

namespace pingus {


class Playfield;
class Server;
class Vector3f;
class SmallMapImage;

/** This is the map that appears in the corner of the screen */
class SmallMap : public gui::RectComponent
{
private:
  Server*    server;
  Playfield* playfield;

  Sprite exit_sur;
  Sprite entrance_sur;

  std::unique_ptr<SmallMapImage> image;

  bool scroll_mode;
  bool has_focus;

  /** Pointer to the current GC, only valid inside draw() */
  DrawingContext* gc_ptr;

  // Camera box drawn by compositing terrain + border pixels into a single
  // surface that is uploaded as one texture and blitted via SDL_RenderCopy.
  // A composite blit scales all pixels as a unit; the 1-pixel green border
  // is never blitted independently so it is immune to the SDL fractional-
  // scale integer-truncation that causes individual 1-pixel primitives to
  // vanish at 0.8x scale (640x480 physical / 800x600 logical).
  Surface      m_composite;          // CPU surface: terrain + camera border
  Sprite       m_composite_sprite;   // GPU sprite uploaded from m_composite
  int          m_last_view_left;
  int          m_last_view_top;
  unsigned int m_last_colmap_serial;

public:
  SmallMap(Server*, Playfield*, const Rect& rect);
  virtual ~SmallMap();

  void on_primary_button_press(int x, int y);
  void on_primary_button_release(int x, int y);
  void on_pointer_move(int x, int y);

  void on_pointer_enter();
  void on_pointer_leave();

  bool is_at(int x, int y);
  bool mouse_over();

  void draw(DrawingContext& gc);
  void update(float delta);

  void draw_sprite(Sprite sprite, Vector3f pos);

private:
  /** Rebuild m_composite from the collision map and bake in the camera box. */
  void rebuild_composite(const Rect& view_rect);

  SmallMap(const SmallMap&);
  SmallMap& operator=(const SmallMap&);
};


} // namespace pingus
#endif

// EOF
