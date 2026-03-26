// src/pingus/worldobj.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1999 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/worldobj.hpp"

#include "util/log.hpp"

namespace pingus {


World* WorldObj::world;

void
WorldObj::set_world(World* arg_world)
{
  world = arg_world;
}

WorldObj::WorldObj(const FileReader& reader) :
  id()
{
  reader.read_string("id", id);
}

WorldObj::WorldObj() :
  id()
{
  // z_pos = 0;
}

WorldObj::~WorldObj()
{

}

void
WorldObj::on_startup()
{
  // do nothing
}

void
WorldObj::update()
{
  // do nothing
}

void
WorldObj::draw_smallmap(SmallMap* /*smallmap*/)
{
}

bool
WorldObj::is_solid_background() const
{
  return false;
}


} // namespace pingus

// EOF
