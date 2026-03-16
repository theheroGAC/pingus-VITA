// src/math/color.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2005 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_MATH_COLOR_HPP
#define HEADER_PINGUS_MATH_COLOR_HPP

#include <SDL2/SDL.h>

namespace pingus {

class Color
{
public:
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;

  constexpr Color() noexcept
    : r(0), g(0), b(0), a(255)
  {}

  constexpr Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255) noexcept
    : r(r_), g(g_), b(b_), a(a_)
  {}

  constexpr bool operator==(Color rhs) const noexcept {
    return
      r == rhs.r &&
      g == rhs.g &&
      b == rhs.b &&
      a == rhs.a;
  }
};



class Colorf
{
public:
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  float a = 1.0f;

  constexpr Color to_color() const noexcept {
    return Color(static_cast<uint8_t>(255 * r),
                 static_cast<uint8_t>(255 * g),
                 static_cast<uint8_t>(255 * b),
                 static_cast<uint8_t>(255 * a));
  }
};

} // namespace pingus
#endif

// EOF
