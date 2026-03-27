// src/pingus/worldmap/dot.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2002 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/worldmap/dot.hpp"


namespace pingus::worldmap {

Dot::Dot(const FileReader& reader) :
  Drawable(),
  pos(),
  m_highlight(false)
{
  reader.read_vector("position", pos);
  reader.read_string("name",     name);

  assert(!name.empty());
}

} // namespace pingus::worldmap

// EOF
