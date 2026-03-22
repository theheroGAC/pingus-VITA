// src/engine/display/sdl_framebuffer_surface_impl.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_DISPLAY_SDL_FRAMEBUFFER_SURFACE_IMPL_HPP
#define HEADER_PINGUS_ENGINE_DISPLAY_SDL_FRAMEBUFFER_SURFACE_IMPL_HPP

#include "engine/display/framebuffer_surface.hpp"

namespace pingus {

class SDLFramebufferSurfaceImpl : public FramebufferSurfaceImpl
{
private:
  SDL_Texture* m_texture;
  // Dimensions as seen by the game -- always the ORIGINAL source dimensions.
  // All sprite frame calculations, worldmap positioning, etc. rely on these.
  int m_width;
  int m_height;
  // If the source was too large for the hardware and had to be scaled down,
  // these factors map original pixel coords -> actual texture coords.
  // For most textures (no scaling) these are 1.0f.
  float m_scale_x;
  float m_scale_y;

public:
  SDLFramebufferSurfaceImpl(SDL_Renderer* renderer, SDL_Surface* src);
  ~SDLFramebufferSurfaceImpl();

  int   get_width()   const { return m_width;   }
  int   get_height()  const { return m_height;  }
  float get_scale_x() const { return m_scale_x; }
  float get_scale_y() const { return m_scale_y; }

  SDL_Texture* get_texture() const { return m_texture; }

private:
  SDLFramebufferSurfaceImpl(const SDLFramebufferSurfaceImpl&);
  SDLFramebufferSurfaceImpl& operator=(const SDLFramebufferSurfaceImpl&);
};


} // namespace pingus
#endif

// EOF
