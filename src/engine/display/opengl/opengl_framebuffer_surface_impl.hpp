// src/engine/display/opengl/opengl_framebuffer_surface_impl.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_DISPLAY_OPENGL_OPENGL_FRAMEBUFFER_SURFACE_IMPL_HPP
#define HEADER_PINGUS_ENGINE_DISPLAY_OPENGL_OPENGL_FRAMEBUFFER_SURFACE_IMPL_HPP

#if defined(__VITA__)
#  include <vitaGL.h>
#elif defined(__WII__)
#  include <GL/gl.h>
#else
#  include <SDL_opengl.h>
#endif

#include <vector>

#include "engine/display/framebuffer_surface.hpp"
#include "math/rect.hpp"

namespace pingus {


struct OpenGLTile {
  GLuint handle;
  Rect   rect;         // Position and size in the original image
  Size   texture_size; // Actual allocated size of the GL texture (POT or aligned)
  GLfloat u_scale;     // Pre-calculated 1.0f / texture_size.width
  GLfloat v_scale;     // Pre-calculated 1.0f / texture_size.height
};

class OpenGLFramebufferSurfaceImpl : public FramebufferSurfaceImpl
{
private:
  std::vector<OpenGLTile> m_tiles;
  Size m_size;

public:
  OpenGLFramebufferSurfaceImpl(SDL_Surface* src);
  ~OpenGLFramebufferSurfaceImpl();

  int get_width()  const { return m_size.width;  }
  int get_height() const { return m_size.height; }

  const std::vector<OpenGLTile>& get_tiles() const { return m_tiles; }
  Size get_size() const { return m_size; }

private:
  OpenGLFramebufferSurfaceImpl(const OpenGLFramebufferSurfaceImpl&);
  OpenGLFramebufferSurfaceImpl& operator=(const OpenGLFramebufferSurfaceImpl&);
};


} // namespace pingus
#endif

// EOF
