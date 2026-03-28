// src/engine/display/sprite_impl.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2005-2008 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_DISPLAY_SPRITE_IMPL_HPP
#define HEADER_PINGUS_ENGINE_DISPLAY_SPRITE_IMPL_HPP

#include "engine/display/framebuffer_surface.hpp"
#include "math/vector2i.hpp"

namespace pingus {

class SpriteDescription;
class Framebuffer;

class SpriteImpl
{
private:
  friend class Sprite;

  Pathname filename;
  FramebufferSurface framebuffer_surface;

  Vector2i offset;

  Vector2i frame_pos;
  Size     frame_size;
  int      frame_delay;

  Size     array;

  bool     loop;
  bool     loop_last_cycle;
  bool     finished;

  /** Current frame */
  int      frame;
  int      tick_count;
  int      total_time;

public:
  SpriteImpl();
  SpriteImpl(const SpriteDescription& desc, ResourceModifier::Enum mod = ResourceModifier::ROT0);
  SpriteImpl(const Surface& surface_);
  ~SpriteImpl();

  static void purge_surface_cache();

  void update(float delta);

  void render(int x, int y, Framebuffer& fb);

  void restart();
  void finish();
};


} // namespace pingus
#endif

// EOF
