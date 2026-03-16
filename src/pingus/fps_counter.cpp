// src/pingus/fps_counter.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2000 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/fps_counter.hpp"

#include <SDL2/SDL.h>
#include <sstream>

#include "engine/display/display.hpp"
#include "pingus/fonts.hpp"

namespace pingus {


FPSCounter fps_counter;

FPSCounter::FPSCounter() :
  fps_string(),
  fps_count(),
  start_time()
{
  start_time = SDL_GetTicks();
  fps_count = 0;
}

FPSCounter::~FPSCounter()
{
}

void
FPSCounter::draw()
{
  update_fps_counter();
  fonts::pingus_small.render(origin_center, Display::get_width()/2, 35, fps_string, *Display::get_framebuffer());
}

void
FPSCounter::update_fps_counter()
{
  unsigned int current_time = SDL_GetTicks();
  int current_fps;

  fps_count++;

  if (start_time + 1000 < current_time)
  {
    current_fps = fps_count * 1000 / (current_time - start_time);

    fps_count = 0;
    start_time = SDL_GetTicks();

    std::ostringstream str;
    str << current_fps << " fps";
    fps_string = str.str();
  }
}


} // namespace pingus

// EOF
