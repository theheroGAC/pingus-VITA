// src/engine/system/sdl_system.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1998-2011 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_SYSTEM_SDL_HPP
#define HEADER_PINGUS_ENGINE_SYSTEM_SDL_HPP

#include <SDL2/SDL.h>

#include "pingus/options.hpp"

namespace pingus {


class Size;

class SDLSystem
{
public:
  SDLSystem();
  ~SDLSystem();

  void create_window(FramebufferType framebuffer_type, const Size& size, bool fullscreen, bool resizable);

private:
  SDLSystem(const SDLSystem&);
  SDLSystem& operator=(const SDLSystem&);
};


} // namespace pingus
#endif

// EOF
