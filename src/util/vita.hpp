// src/util/vita.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2026 theheroGAC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_UTIL_VITA_HPP
#define HEADER_PINGUS_UTIL_VITA_HPP

#ifdef __VITA__

#include <string>

namespace pingus {

namespace Vita {

std::string get_base_dir();
std::string get_data_dir();

} // namespace Vita

} // namespace pingus

#endif // __VITA__

#endif // HEADER_PINGUS_UTIL_VITA_HPP

// EOF
