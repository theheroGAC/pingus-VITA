// src/engine/display/blitter.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1999 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_DISPLAY_BLITTER_HPP
#define HEADER_PINGUS_ENGINE_DISPLAY_BLITTER_HPP

#include <SDL2/SDL.h>

namespace pingus {

class Surface;

/** A bunch of blitting and creation functions to operate on Surface.  */
class Blitter
{
public:
  static SDL_Surface* create_surface_rgba(int w, int h);
  static SDL_Surface* create_surface_rgb(int w, int h);
  static SDL_Surface* create_surface_from_format(SDL_Surface* surface, int w, int h);

  /** Flip a surface horizontal */
  static Surface flip_horizontal (Surface sur);

  /** Flip a surface vertical */
  static Surface flip_vertical (Surface sur);

  /** Rotate a surface 90 degrees */
  static Surface rotate_90 (Surface sur);

  static Surface rotate_180 (Surface sur);

  static Surface rotate_270 (Surface sur);

  static Surface rotate_90_flip (Surface sur);

  static Surface rotate_180_flip (Surface sur);

  static Surface rotate_270_flip (Surface sur);

  /** Creates a new surface with the given width and height and
      stretches the source surface onto it, the caller is responsible
      to delete the returned Surface.

      @param surface The source surface
      @param width The new width of the surface.
      @param height The new height of the surface.
      @return A newly created surface, the caller is responsible to delete it. */
  static SDL_Surface* scale_surface(SDL_Surface* surface, int width, int height);

private:
  Blitter (const Blitter&);
  Blitter& operator= (const Blitter&);
};


} // namespace pingus
#endif

// EOF
