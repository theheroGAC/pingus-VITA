// src/pingus/worldmap/dot.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2002 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_PINGUS_WORLDMAP_DOT_HPP
#define HEADER_PINGUS_PINGUS_WORLDMAP_DOT_HPP

#include "math/vector3f.hpp"
#include "pingus/worldmap/drawable.hpp"

namespace pingus::worldmap {

/** A Dot is a node between all the pathes on the worldmap, there are
    LevelDots TubeDots and other availabe. */
class Dot : public Drawable
{
protected:
  Vector3f pos;

public:
  Dot(const FileReader& reader);

  /** Draw stuff that should be displayed if the mouse is over the dot */
  virtual void draw_hover(DrawingContext& gc) =0;

  Vector3f get_pos() { return pos; }

  virtual void on_click() =0;

  virtual bool is_finished() const =0;
  virtual bool is_accessible() const =0;
  /** makes the node accessible */
  virtual void unlock() =0;

  bool get_highlight() const { return m_highlight; }
  void set_highlight(bool highlight) { m_highlight = highlight; }

protected:
  bool m_highlight;

private:
  Dot (const Dot&);
  Dot& operator= (const Dot&);
};

} // namespace pingus::worldmap

#endif

// EOF
