// src/engine/display/blitter.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1999 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "engine/display/blitter.hpp"
#include "engine/display/blitter_impl.hpp"

#include <cassert>

#include "engine/display/surface.hpp"

namespace pingus {


SDL_Surface*
Blitter::scale_surface(SDL_Surface* surface, int width, int height)
{
  assert(surface != nullptr);

  SDL_Surface* new_surface;

  if (surface->format->palette)
  {
    // Paletted (indexed colour) surface
    Uint32 colorkey = 0;
    bool useckey = (SDL_GetColorKey(surface, &colorkey) == 0);

    new_surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
    if (useckey)
      SDL_SetColorKey(new_surface, SDL_TRUE, colorkey);

    SDL_LockSurface(surface);
    SDL_LockSurface(new_surface);

    float xscale = static_cast<float>(surface->w) / static_cast<float>(width);
    float yscale = static_cast<float>(surface->h) / static_cast<float>(height);

    for (int y = 0; y < new_surface->h; ++y)
      for (int x = 0; x < new_surface->w; ++x)
      {
        Uint8* src = static_cast<Uint8*>(surface->pixels)
                     + (static_cast<int>(static_cast<float>(y) * yscale) * surface->pitch)
                     + static_cast<int>(static_cast<float>(x) * xscale);
        Uint8* dst = static_cast<Uint8*>(new_surface->pixels) + y * new_surface->pitch + x;
        *dst = *src;
      }

    SDL_UnlockSurface(surface);
    SDL_UnlockSurface(new_surface);

    if (surface->format->palette && new_surface->format->palette)
      SDL_SetPaletteColors(new_surface->format->palette,
                           surface->format->palette->colors,
                           0,
                           surface->format->palette->ncolors);
  }
  else
  {
    new_surface = SDL_CreateRGBSurface(0, width, height,
                                       surface->format->BitsPerPixel,
                                       surface->format->Rmask,
                                       surface->format->Gmask,
                                       surface->format->Bmask,
                                       surface->format->Amask);

    SDL_LockSurface(surface);
    SDL_LockSurface(new_surface);

    float xscale = static_cast<float>(surface->w) / static_cast<float>(width);
    float yscale = static_cast<float>(surface->h) / static_cast<float>(height);

    for (int y = 0; y < new_surface->h; ++y)
      for (int x = 0; x < new_surface->w; ++x)
      {
        Uint8* src = static_cast<Uint8*>(surface->pixels)
                     + (static_cast<int>(static_cast<float>(y) * yscale) * surface->pitch)
                     + (static_cast<int>(static_cast<float>(x) * xscale) * surface->format->BytesPerPixel);
        Uint8* dst = static_cast<Uint8*>(new_surface->pixels)
                     + y * new_surface->pitch
                     + x * new_surface->format->BytesPerPixel;
        memcpy(dst, src, surface->format->BytesPerPixel);
      }

    SDL_UnlockSurface(surface);
    SDL_UnlockSurface(new_surface);
  }

  return new_surface;
}

SDL_Surface*
Blitter::create_surface_rgba(int w, int h)
{
  return SDL_CreateRGBSurface(0, w, h, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                              0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
                              0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
    );
}

SDL_Surface*
Blitter::create_surface_rgb(int w, int h)
{
  return SDL_CreateRGBSurface(0, w, h, 24,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                              0xff0000, 0x00ff00, 0x0000ff, 0x000000
#else
                              0x0000ff, 0x00ff00, 0xff0000, 0x000000
#endif
    );
}

SDL_Surface*
Blitter::create_surface_from_format(SDL_Surface* surface, int w, int h)
{
  SDL_Surface* new_surface = SDL_CreateRGBSurface(0, w, h,
                              surface->format->BitsPerPixel,
                              surface->format->Rmask,
                              surface->format->Gmask,
                              surface->format->Bmask,
                              surface->format->Amask);

  // Propagate palette (for indexed/paletted surfaces)
  if (surface->format->palette && new_surface->format->palette)
    SDL_SetPaletteColors(new_surface->format->palette,
                         surface->format->palette->colors,
                         0, surface->format->palette->ncolors);

  // Propagate colorkey
  Uint32 colorkey = 0;
  if (SDL_GetColorKey(surface, &colorkey) == 0)
    SDL_SetColorKey(new_surface, SDL_TRUE, colorkey);

  // Propagate blend mode (SDL2 equivalent of SDL_SRCALPHA)
  // Without this, surfaces with transparency render as solid white blocks.
  SDL_BlendMode blend;
  SDL_GetSurfaceBlendMode(surface, &blend);
  SDL_SetSurfaceBlendMode(new_surface, blend);

  return new_surface;
}

/** Flip a surface horizontal */
Surface
Blitter::flip_horizontal (Surface prov)
{
  return BlitterImpl::modify<BlitterImpl::transform_flip>(prov);
}

/** Flip a surface vertical */
Surface
Blitter::flip_vertical (Surface sur)
{
  return BlitterImpl::modify<BlitterImpl::transform_rot180_flip>(sur);
}

/** Rotate a surface 90 degrees */
Surface
Blitter::rotate_90 (Surface sur)
{
  return BlitterImpl::modify<BlitterImpl::transform_rot90>(sur);
}

Surface
Blitter::rotate_180 (Surface sur)
{
  return BlitterImpl::modify<BlitterImpl::transform_rot180>(sur);
}

Surface
Blitter::rotate_270 (Surface sur)
{
  return BlitterImpl::modify<BlitterImpl::transform_rot270>(sur);
}

Surface
Blitter::rotate_90_flip (Surface sur)
{
  return BlitterImpl::modify<BlitterImpl::transform_rot90_flip>(sur);
}

Surface
Blitter::rotate_180_flip (Surface sur)
{
  return BlitterImpl::modify<BlitterImpl::transform_rot180_flip>(sur);
}

Surface
Blitter::rotate_270_flip (Surface sur)
{
  return BlitterImpl::modify<BlitterImpl::transform_rot270_flip>(sur);
}


} // namespace pingus

// EOF
