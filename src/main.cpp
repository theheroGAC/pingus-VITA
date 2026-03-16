// src/main.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1998-2009 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include <SDL2/SDL.h>

#include "pingus/pingus_main.hpp"

int main(int argc, char** argv)
{
  pingus::PingusMain app;
  return app.run(argc, argv);
}

// EOF
