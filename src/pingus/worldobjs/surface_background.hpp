// src/pingus/worldobjs/surface_background.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1999 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_PINGUS_WORLDOBJS_SURFACE_BACKGROUND_HPP
#define HEADER_PINGUS_PINGUS_WORLDOBJS_SURFACE_BACKGROUND_HPP

#include "math/vector3f.hpp"
#include "math/color.hpp"
#include "pingus/worldobj.hpp"

namespace pingus::worldobjs {

class SurfaceBackground : public WorldObj
{
private:
  float para_x;
  float para_y;

  /** Position of the background, only the z_pos is really used. */
  Vector3f pos;

  /** The amount of pixel the background is scrolled each frame in x
      direction. */
  float scroll_x;

  /** The amount of pixel the background is scrolled each frame in x
      direction. */
  float scroll_y;

  /** fill_rect() components
      An fill_rect() can be drawn over the background, the following
      for components are passed to the fill_rect() call. */
  Color color;

  /** Stretch the background to the full screen size in x direction */
  bool stretch_x;

  /** Stretch the background to the full screen size in x direction */
  bool stretch_y;

  /** If streched in x or y direction keep the aspect ratio */
  bool keep_aspect;

  /** Background image */
  Sprite bg_sprite;

  /** The horizontal scrolling speed in pixels per tick */
  float scroll_ox;

  /** The vertical scrolling speed in pixels per tick */
  float scroll_oy;

public:
  SurfaceBackground(const FileReader& reader);

  float get_z_pos () const;
  void set_pos(Vector3f /*p*/) { }
  Vector3f get_pos() const { return Vector3f(); }

  void update ();
  void draw (SceneContext& gc);

  bool is_solid_background() const override { return true; }

private:
  SurfaceBackground (const SurfaceBackground&);
  SurfaceBackground& operator= (const SurfaceBackground&);
};

} // namespace pingus::worldobjs

#endif

// EOF
