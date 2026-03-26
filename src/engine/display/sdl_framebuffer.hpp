// src/engine/display/sdl_framebuffer.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_DISPLAY_SDL_FRAMEBUFFER_HPP
#define HEADER_PINGUS_ENGINE_DISPLAY_SDL_FRAMEBUFFER_HPP

#include "engine/display/framebuffer.hpp"

namespace pingus {

class SDLFramebuffer : public Framebuffer
{
private:
  SDL_Window*   m_window;        // physical window
  SDL_Renderer* m_renderer;      // SDL2 hardware-accelerated renderer
  SDL_Texture*  m_pixel_texture; // 1x1 texture for drawing lines
  bool m_fullscreen;
  bool m_resizable;
  // Clip rect stack in logical coordinates.
  // SDL_RenderSetClipRect scales these itself, same as all other draw calls.
  std::vector<Rect> cliprect_stack;

public:
  SDLFramebuffer();
  ~SDLFramebuffer();

  FramebufferSurface create_surface(const Surface& surface);

  void set_video_mode(const Size& size, bool fullscreen, bool resizable);
  bool is_fullscreen() const;
  bool is_resizable() const;
  void flip();
  void update_rects(const std::vector<Rect>& rects);

  void push_cliprect(const Rect&);
  void pop_cliprect();

  void draw_surface(const FramebufferSurface& src, Vector2i pos);
  void draw_surface(const FramebufferSurface& src, const Rect& srcrect, Vector2i pos);

  void draw_line(Vector2i pos1, Vector2i pos2, Color color);

  void draw_rect(const Rect& rect, Color color);
  void fill_rect(const Rect& rect, Color color);

  Size get_size() const;

  SDL_Window* get_window() const override { return m_window; }

private:
  SDLFramebuffer(const SDLFramebuffer&);
  SDLFramebuffer& operator=(const SDLFramebuffer&);
};


} // namespace pingus
#endif

// EOF
