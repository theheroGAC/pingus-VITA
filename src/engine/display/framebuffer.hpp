// src/engine/display/framebuffer.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_DISPLAY_FRAMEBUFFER_HPP
#define HEADER_PINGUS_ENGINE_DISPLAY_FRAMEBUFFER_HPP

#include <SDL2/SDL.h>
#include <vector>

#include "engine/display/framebuffer_surface.hpp"
#include "math/color.hpp"
#include "math/rect.hpp"
#include "math/size.hpp"
#include "math/vector2i.hpp"

namespace pingus {

class Surface;

class Framebuffer
{
public:
  Framebuffer() {}
  virtual ~Framebuffer() {}

  virtual FramebufferSurface create_surface(const Surface& surface) =0;

  virtual void set_video_mode(const Size& size, bool fullscreen, bool resizable) =0;
  virtual bool is_fullscreen() const =0;
  virtual bool is_resizable() const =0;
  virtual void flip() =0;

  virtual void push_cliprect(const Rect&) =0;
  virtual void pop_cliprect() =0;

  virtual void draw_surface(const FramebufferSurface& src, Vector2i pos) =0;
  virtual void draw_surface(const FramebufferSurface& src, const Rect& srcrect, Vector2i pos) =0;

  virtual void draw_line(Vector2i pos1, Vector2i pos2, Color color) =0;

  virtual void draw_rect(const Rect& rect, Color color) =0;
  virtual void fill_rect(const Rect& rect, Color color) =0;

  virtual Size get_size() const =0;

  /** Returns the underlying SDL_Window, or nullptr for non-SDL backends.
   *  Used by SDLSystem to set window caption/icon after window creation,
   *  and by Screenshot to read back pixels from the SDL framebuffer. */
  virtual SDL_Window* get_window() const { return nullptr; }

  /** Returns the software render surface (SDL framebuffer only).
   *  Used by Screenshot::make_screenshot() to read pixels without OpenGL. */
  virtual SDL_Surface* get_sdl_surface() const { return nullptr; }
};


} // namespace pingus
#endif

// EOF
