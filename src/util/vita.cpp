// src/util/vita.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2026 theheroGAC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifdef __VITA__

#include "util/vita.hpp"
#include "util/system.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <stdexcept>
#include <string>

namespace pingus {

namespace Vita {

std::string get_base_dir()
{
  return "ux0:data/pingus/";
}

std::string get_data_dir()
{
  return "ux0:data/pingus/";
}

} // namespace Vita

} // namespace pingus

#endif // __VITA__

// EOF
