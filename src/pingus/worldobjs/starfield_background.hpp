// src/pingus/worldobjs/starfield_background.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2000 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_PINGUS_WORLDOBJS_STARFIELD_BACKGROUND_HPP
#define HEADER_PINGUS_PINGUS_WORLDOBJS_STARFIELD_BACKGROUND_HPP

#include <vector>

#include "pingus/worldobj.hpp"

namespace pingus::worldobjs {

class StarfieldBackgroundStars;

class StarfieldBackground : public WorldObj
{
private:
  int  small_stars_count;
  int middle_stars_count;
  int  large_stars_count;

  std::vector<StarfieldBackgroundStars*>        stars;

public:
  StarfieldBackground(const FileReader& reader);
  ~StarfieldBackground();

  // FIXME: Make z_pos handling editable via xml
  float get_z_pos() const { return -10; }
  void set_pos(Vector3f /*p*/) {}
  Vector3f get_pos() const { return Vector3f(); }

  void update ();
  void draw (SceneContext& gc);

  bool is_solid_background() const override { return true; }

private:
  StarfieldBackground (const StarfieldBackground&);
  StarfieldBackground& operator= (const StarfieldBackground&);
};

} // namespace pingus::worldobjs

#endif

// EOF
