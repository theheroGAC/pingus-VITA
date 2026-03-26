// src/pingus/worldobjs/solid_color_background.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2000 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_PINGUS_WORLDOBJS_SOLID_COLOR_BACKGROUND_HPP
#define HEADER_PINGUS_PINGUS_WORLDOBJS_SOLID_COLOR_BACKGROUND_HPP

#include "math/color.hpp"
#include "pingus/worldobj.hpp"

namespace pingus::worldobjs {

class SolidColorBackground : public WorldObj
{
private:
  Color color;

public:
  SolidColorBackground(const FileReader& reader);

  // FIXME: Make z_position editable
  float get_z_pos () const { return -10; }
  void set_pos(Vector3f /*p*/) {}
  Vector3f get_pos() const { return Vector3f(); }

  void update () {}

  void draw (SceneContext& gc);

  bool is_solid_background() const override { return true; }

private:
  SolidColorBackground (const SolidColorBackground&);
  SolidColorBackground& operator= (const SolidColorBackground&);
};

} // namespace pingus::worldobjs

#endif

// EOF
