// src/pingus/movers/linear_mover.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1999 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/movers/linear_mover.hpp"

#include "pingus/collider.hpp"

namespace pingus::movers {

LinearMover::LinearMover(World* const world_arg, const Vector3f& pos_arg)
  : Mover(world_arg, pos_arg)
{
}

LinearMover::~LinearMover()
{
}

void LinearMover::update(const Vector3f& move, const Collider& collision_check)
{
  Vector3f step_vector = move;

  // Static cast to stop warning
  int move_length = static_cast<int>(move.length());

  // Make the step vector (i.e. change to a unit vector)
  step_vector.normalize();

  collision = false;

  // Move to the destination one unit vector at a time
  for (int i = 0; i < move_length && !collision; ++i)
  {
    collision = collision_check(world, pos, step_vector);

    pos += step_vector;
  }

  // If on a collision pixel, back away from it.
  if (collision)
    pos -= step_vector;
}

} // namespace pingus::movers

// EOF
