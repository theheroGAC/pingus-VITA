// src/util/string_util.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2005 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_UTIL_STRING_UTIL_HPP
#define HEADER_PINGUS_UTIL_STRING_UTIL_HPP

#include <sstream>
#include <string>
#include <type_traits>

namespace pingus {


class StringUtil
{
private:
public:
  static std::string to_lower(const std::string &str);
  static std::string to_upper(const std::string &str);

  template<class T>
  static T to(const std::string& s, const T& val = T())
  {
    T tmp = val;
    std::istringstream str(s);
    str >> tmp;
    return tmp;
  }

  template<class T>
  static bool from_string(const std::string& s, T& t)
  {
    std::istringstream str(s);
    T tmp;
    if (str >> tmp && (str >> std::ws).eof())
    {
      t = tmp;
      return true;
    }
    return false;
  }

  template<class T>
  static std::string to_string(const T& t)
  {
    if constexpr (std::is_arithmetic_v<T>)
    {
      return std::to_string(t);
    }
    else
    {
      std::ostringstream ss;
      ss << t;
      return ss.str();
    }
  }


private:
  StringUtil ();
  StringUtil (const StringUtil&);
  StringUtil& operator= (const StringUtil&);
};


} // namespace pingus
#endif

// EOF
