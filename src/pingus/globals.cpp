// src/pingus/globals.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1999 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/globals.hpp"

namespace pingus {

namespace globals {

int         game_speed              = 20;
float       desired_fps             = 40;
bool        print_fps               = false;
bool        music_enabled           = true;
bool        sound_enabled           = true;
int         fast_forward_time_scale = 4;
bool        developer_mode          = false;
bool        auto_scrolling          = true;
bool        drag_drop_scrolling     = false;
int         tile_size               = 32;


bool        draw_collision_map      = false;

#if defined(__WII__) || defined(__VITA__)
bool        software_cursor         = true;
#else
bool        software_cursor         = false;
#endif

std::string global_username;
std::string global_email;
std::string default_language        = "en";

} // namespace globals

} // namespace pingus

// EOF
