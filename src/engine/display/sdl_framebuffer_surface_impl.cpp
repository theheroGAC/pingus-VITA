// src/engine/display/sdl_framebuffer_surface_impl.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "engine/display/sdl_framebuffer_surface_impl.hpp"

namespace pingus {


SDLFramebufferSurfaceImpl::SDLFramebufferSurfaceImpl(SDL_Surface* src) :
  surface()
{
  // In SDL2, SDL_DisplayFormat / SDL_DisplayFormatAlpha no longer exist.
  // We use SDL_ConvertSurfaceFormat instead.
  // Pick RGBA8888 when the source has an alpha channel or a colorkey,
  // otherwise use RGB24 (no alpha).
  Uint32 colorkey = 0;
  bool has_colorkey = (SDL_GetColorKey(src, &colorkey) == 0);

  if (src->format->Amask != 0 || has_colorkey)
    surface = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_ARGB8888, 0);
  else
    surface = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGB24, 0);
}

SDLFramebufferSurfaceImpl::~SDLFramebufferSurfaceImpl()
{
  SDL_FreeSurface(surface);
}


} // namespace pingus

// EOF
