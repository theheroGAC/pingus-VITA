// src/engine/display/drawing_request.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2002 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_DISPLAY_DRAWING_REQUEST_HPP
#define HEADER_PINGUS_ENGINE_DISPLAY_DRAWING_REQUEST_HPP

#include <SDL2/SDL.h>

#include "math/rect.hpp"
#include "math/vector2i.hpp"

namespace pingus {

class Framebuffer;

class DrawingRequest
{
protected:
  Vector2i pos;
  float    z;

public:
  DrawingRequest(const Vector2i& pos_, float z_) : pos(pos_), z(z_) {}
  virtual ~DrawingRequest() {};

  /** \a rect is the rectangle that is managed by the parent
      DrawingContext, all calls to fb have to be offset with
      (rect.left,rect.top)  */
  virtual void render(Framebuffer& fb, const Rect& rect) = 0;

  /** Returns true if the request contains an alpha channel and needs
      to be drawn in order */
  virtual bool has_alpha() { return true; }

  /** Returns the position at which the request should be drawn */
  virtual float get_z_pos() { return z; }

private:
  DrawingRequest (const DrawingRequest&);
  DrawingRequest& operator= (const DrawingRequest&);
};


} // namespace pingus
#endif

// EOF
