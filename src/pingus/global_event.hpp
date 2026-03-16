// src/pingus/global_event.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2000 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_PINGUS_GLOBAL_EVENT_HPP
#define HEADER_PINGUS_PINGUS_GLOBAL_EVENT_HPP

#include <SDL2/SDL.h>

namespace pingus {

class GlobalEvent
{
public:
  GlobalEvent ();
  virtual ~GlobalEvent() {}

  virtual void on_button_press(const   SDL_KeyboardEvent& event);
  virtual void on_button_release(const SDL_KeyboardEvent& event);

private:
  GlobalEvent (const GlobalEvent&);
  GlobalEvent& operator= (const GlobalEvent&);
};

extern GlobalEvent global_event;


} // namespace pingus
#endif

// EOF
