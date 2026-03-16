// src/engine/display/opengl/opengl_framebuffer.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_DISPLAY_OPENGL_OPENGL_FRAMEBUFFER_HPP
#define HEADER_PINGUS_ENGINE_DISPLAY_OPENGL_OPENGL_FRAMEBUFFER_HPP

#include "engine/display/framebuffer.hpp"

#ifdef __WII__
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glext.h>
#else
#  include <SDL2/SDL_opengl.h>
#endif

#include <vector>

namespace pingus {


class OpenGLFramebuffer : public Framebuffer
{
private:
  SDL_Window*    m_window;
  SDL_GLContext  m_gl_context;
  int            m_window_w;
  int            m_window_h;
  std::vector<Rect> cliprect_stack;

  // State caching to avoid redundant OpenGL calls
  GLuint m_last_texture_id;
  bool m_texture_enabled;
  bool m_texcoord_array_enabled;

public:
  OpenGLFramebuffer();
  ~OpenGLFramebuffer();

  FramebufferSurface create_surface(const Surface& surface);

  void set_video_mode(const Size& size, bool fullscreen, bool resizable);
  bool is_fullscreen() const;
  bool is_resizable() const;
  void flip();

  void push_cliprect(const Rect& rect);
  void pop_cliprect();

  void draw_surface(const FramebufferSurface& src, Vector2i pos);
  void draw_surface(const FramebufferSurface& src, const Rect& srcrect, Vector2i pos);

  void draw_line(Vector2i pos1, Vector2i pos2, Color color);
  void draw_rect(const Rect& rect, Color color);
  void fill_rect(const Rect& rect, Color color);

  Size get_size() const;

  SDL_Window* get_window() const override { return m_window; }

  /** Forces the framebuffer to reset its internal state cache and OpenGL state. */
  void invalidate_state();

private:
  OpenGLFramebuffer(const OpenGLFramebuffer&);
  OpenGLFramebuffer& operator=(const OpenGLFramebuffer&);
};


} // namespace pingus
#endif

// EOF
