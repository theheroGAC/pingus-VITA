// src/pingus/worldmap/dot_factory.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2002 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include <format>
#include "pingus/worldmap/dot_factory.hpp"

#include <stdexcept>

#include "pingus/worldmap/level_dot.hpp"
#include "pingus/worldmap/story_dot.hpp"

namespace pingus::worldmap {

std::unique_ptr<Dot>
DotFactory::create(const FileReader& reader)
{
  if (reader.get_name() == "storydot")
  {
    return std::make_unique<StoryDot>(reader);
  }
  else if (reader.get_name() == "leveldot")
  {
    return std::make_unique<LevelDot>(reader);
  }
  else
  {
    throw std::runtime_error(std::format("DotFactory: unknown tag: {}", reader.get_name()));
  }
}

} // namespace pingus::worldmap

// EOF
