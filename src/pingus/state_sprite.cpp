// src/pingus/state_sprite.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2002 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/state_sprite.hpp"

#include <sstream>
#include <stdexcept>


namespace pingus {

StateSprite::StateSprite() :
  sprites()
{
}

void
StateSprite::load(int state, const std::string& name)
{
  load(state, Sprite(name));
}

void
StateSprite::load(int state, Sprite sprite)
{
  sprites[state] = sprite;
}

void
StateSprite::update()
{
  update(0.033f);
}

void
StateSprite::update(float delta)
{
  for(Sprites::iterator i = sprites.begin(); i != sprites.end(); ++i)
    (*i).second.update(delta);
}

Sprite&
StateSprite::operator[](int state)
{
  Sprites::iterator i = sprites.find(state);
  if (i != sprites.end())
  {
    return i->second;
  }
  else
  {
    {
      std::ostringstream ss;
      ss << "StateSprite error: state " << state << " not available";
      throw std::runtime_error(ss.str());
    }
  }
}


} // namespace pingus

// EOF
