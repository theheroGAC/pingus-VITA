// src/pingus/worldmap/level_dot.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2002 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_PINGUS_WORLDMAP_LEVEL_DOT_HPP
#define HEADER_PINGUS_PINGUS_WORLDMAP_LEVEL_DOT_HPP

#include "engine/display/sprite.hpp"
#include "pingus/pingus_level.hpp"
#include "pingus/worldmap/dot.hpp"

namespace pingus::worldmap {

class LevelDot : public Dot
{
private:
  Sprite green_dot_sur;
  Sprite red_dot_sur;
  Sprite inaccessible_dot_sur;
  Sprite highlight_green_dot_sur;
  Sprite highlight_red_dot_sur;

  PingusLevel plf;

public:
  LevelDot(const FileReader& reader);

  void draw(DrawingContext& gc);
  void draw_hover(DrawingContext& gc);

  void update(float delta);
  PingusLevel get_plf () const { return plf; }
  void on_click();

  bool is_finished() const override;
  bool is_accessible() const override;
  void unlock() override;

private:
  LevelDot (const LevelDot&);
  LevelDot& operator= (const LevelDot&);
};

} // namespace pingus::worldmap

#endif

// EOF
