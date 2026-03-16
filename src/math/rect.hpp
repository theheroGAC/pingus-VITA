// src/math/rect.hpp
// SPDX-License-Identifier: Zlib
//
// ClanLib SDK
// Copyright (c) 1997-2005 The ClanLib Team
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// File Author(s):
//   Magnus Norddahl
//   (if your name is missing here, please add it)

#ifndef HEADER_PINGUS_MATH_RECT_HPP
#define HEADER_PINGUS_MATH_RECT_HPP

#include <algorithm>
#include <cmath>

#include "math/origin.hpp"
#include "math/vector2f.hpp"

namespace pingus {


class Rectf;

//: 2D (left,top,right,bottom) rectangle structure.
//- !group=Core/Math!
//- !header=core.h!
class Rect
{
  //! Construction:
public:
  //: Constructs an rectangle.
  //param left: Initial left position of rectangle.
  //param top: Initial top position of rectangle.
  //param right: Initial right position of rectangle.
  //param bottom: Initial bottom position of rectangle.
  //param point: Initial top-left position of rectangle.
  //param size: Initial size of rectangle.
  //param rect: Initial rectangle position and size.
  constexpr Rect() noexcept
    : left(0),
      top(0),
      right(0),
      bottom(0)
  {}

  Rect(const Rect&) = default;
  Rect& operator=(const Rect&) = default;

  explicit constexpr Rect(const Rectf& rect) noexcept;

  constexpr Rect(int new_left, int new_top, int new_right, int new_bottom) noexcept
    : left(new_left),
      top(new_top),
      right(new_right),
      bottom(new_bottom)
  {}

  constexpr Rect(const Vector2i &p, const Size &size) noexcept
    : left(p.x),
      top(p.y),
      right(left + size.width),
      bottom(top + size.height)
  {}

  constexpr Rect grow(int b) const noexcept {
    return Rect(left   - b,
                top    - b,
                right  + b,
                bottom + b);
  }

  //: Rect += Rect operator.
  constexpr Rect &operator+=(const Rect &r) noexcept
  { left += r.left; top += r.top; right += r.right; bottom += r.bottom; return *this; }

  //: Rect -= Rect operator.
  constexpr Rect &operator-=(const Rect &r) noexcept
  { left -= r.left; top -= r.top; right -= r.right; bottom -= r.bottom; return *this; }

  //: Rect += Vector2i operator.
  constexpr Rect &operator+=(const Vector2i &p) noexcept
  { left += p.x; top += p.y; right += p.x; bottom += p.y; return *this; }

  //: Rect -= Vector2i operator.
  constexpr Rect &operator-=(const Vector2i &p) noexcept
  { left -= p.x; top -= p.y; right -= p.x; bottom -= p.y; return *this; }

  //: Rect + Rect operator.
  constexpr Rect operator+(const Rect &r) const noexcept
  { return Rect(left + r.left, top + r.top, right + r.right, bottom + r.bottom); }

  //: Rect - Rect operator.
  constexpr Rect operator-(const Rect &r) const noexcept
  { return Rect(left - r.left, top - r.top, right - r.right, bottom - r.bottom); }

  //: Rect + Vector2i operator.
  constexpr Rect operator+(const Vector2i &p) const noexcept
  { return Rect(left + p.x, top + p.y, right + p.x, bottom + p.y); }

  //: Rect - Vector2i operator.
  constexpr Rect operator-(const Vector2i &p) const noexcept
  { return Rect(left - p.x, top - p.y, right - p.x, bottom - p.y); }

  //: Rect == Rect operator.
  constexpr bool operator==(const Rect &r) const noexcept
  { return (left == r.left && top == r.top && right == r.right && bottom == r.bottom); }

  //: Rect != Rect operator.
  constexpr bool operator!=(const Rect &r) const noexcept
  { return (left != r.left || top != r.top || right != r.right || bottom != r.bottom); }

  //! Attributes:
public:
  //: X1-coordinate.
  int left;

  //: Y1-coordinate.
  int top;

  //: X2-coordinate.
  int right;

  //: Y2-coordinate.
  int bottom;

  //: Returns the width of the rectangle.
  constexpr int get_width() const noexcept { return right - left; }

  //: Returns the height of the rectangle.
  constexpr int get_height() const noexcept { return bottom - top; }

  //: Returns the size of the rectangle.
  constexpr Size get_size() const noexcept { return Size(right - left, bottom - top); }

  //: Returns true if rectangle passed is overlapping or inside this rectangle.
  constexpr bool is_overlapped(const Rect &r) const noexcept
  {
    return (r.left < right && r.right > left && r.top < bottom && r.bottom > top);
  }

  constexpr bool contains(const Vector2i& pos) const noexcept
  {
    return
      left <= pos.x && pos.x <= right &&
      top  <= pos.y && pos.y <= bottom;
  }

  //: Check if rect is inside this
  constexpr bool contains(const Rect& rect) const noexcept
  {
    return
      left   <= rect.left  &&
      right  >= rect.right &&
      top    <= rect.top   &&
      bottom >= rect.bottom;
  }

  //: Returns another Rect containing a rotated version of this one.
  //param hotspot: Vector2i to rotate around.
  //param origin: Determines the hotspot point within the rectangle
  //param x, y: Offsets applied negatively to the hotspot point
  //param angle: Angle to rotate in degrees.
  Rect get_rot_bounds(const Vector2i &hotspot, float angle) const;
  Rect get_rot_bounds(Origin origin, int x, int y, float angle) const;

  //! Operations:
public:
  //: Sets the size of the rectangle, maintaining top/left position.
  constexpr void set_size(const Size &size) noexcept
  {
    right = left + size.width;
    bottom = top + size.height;
  }

  //: Calculates and returns the union of two rectangles.
  constexpr Rect calc_union(const Rect &rect) noexcept
  {
    Rect result;
    if (left   > rect.left)   result.left   = left;   else result.left   = rect.left;
    if (right  < rect.right)  result.right  = right;  else result.right  = rect.right;
    if (top    > rect.top)    result.top    = top;    else result.top    = rect.top;
    if (bottom < rect.bottom) result.bottom = bottom; else result.bottom = rect.bottom;
    return result;
  }

  constexpr bool is_normal() const noexcept
  {
    return left <= right && top <= bottom;
  }

  //: Normalize rectangle. Ensures that left is less than right and top is less than bottom.
  constexpr void normalize() noexcept
  {
    if (left > right)
    {
      int temp = right;
      right = left;
      left = temp;
    }

    if (top > bottom)
    {
      int temp = bottom;
      bottom = top;
      top = temp;
    }
  }

  //: Applies an origin and offset pair to this rectangle
  //param origin: The new origin to adjust to from default upper-left position
  //param x, y: Offsets applied negatively to each corner of the rectangle
  void apply_alignment(Origin origin, int x, int y)
  {
    Vector2i offset = calc_origin(origin, get_size());
    offset.x -= x;
    offset.y -= y;

    left += offset.x;
    top += offset.y;
    right += offset.x;
    bottom += offset.y;
  }
};

//: 2D (left,top,right,bottom) floating point rectangle structure.
class Rectf
{
  //! Construction:
public:
  //: Constructs an rectangle.
  //param left: Initial left position of rectangle.
  //param top: Initial top position of rectangle.
  //param right: Initial right position of rectangle.
  //param bottom: Initial bottom position of rectangle.
  //param point: Initial top-left position of rectangle.
  //param size: Initial size of rectangle.
  //param rect: Initial rectangle position and size.
  constexpr Rectf() noexcept
    : left(0.0f),
      top(0.0f),
      right(0.0f),
      bottom(0.0f)
  {}

  Rectf(const Rectf&) = default;
  Rectf& operator=(const Rectf&) = default;

  explicit constexpr Rectf(const Rect& rect) noexcept
    : left(static_cast<float>(rect.left)),
      top(static_cast<float>(rect.top)),
      right(static_cast<float>(rect.right)),
      bottom(static_cast<float>(rect.bottom))
  {}

  constexpr Rectf(float new_left, float new_top, float new_right, float new_bottom) noexcept
    : left(new_left),
      top(new_top),
      right(new_right),
      bottom(new_bottom)
  {}

  constexpr Rectf(const Vector2f &p, const Sizef &size) noexcept
    : left(p.x),
      top(p.y),
      right(left + size.width),
      bottom(top + size.height)
  {}

  constexpr Rectf(const Vector2f& p1, const Vector2f& p2) noexcept
    :  left(p1.x),
       top(p1.y),
       right(p2.x),
       bottom(p2.y)
  {}

  //: Rect += Rect operator.
  constexpr Rectf &operator+=(const Rectf &r) noexcept
  { left += r.left; top += r.top; right += r.right; bottom += r.bottom; return *this; }

  //: Rect -= Rect operator.
  constexpr Rectf &operator-=(const Rectf &r) noexcept
  { left -= r.left; top -= r.top; right -= r.right; bottom -= r.bottom; return *this; }

  //: Rect += Vector2i operator.
  constexpr Rectf &operator+=(const Vector2f &p) noexcept
  { left += p.x; top += p.y; right += p.x; bottom += p.y; return *this; }

  //: Rect -= Vector2i operator.
  constexpr Rectf &operator-=(const Vector2f &p) noexcept
  { left -= p.x; top -= p.y; right -= p.x; bottom -= p.y; return *this; }

  //: Rect + Rect operator.
  constexpr Rectf operator+(const Rectf &r) const noexcept
  { return Rectf(left + r.left, top + r.top, right + r.right, bottom + r.bottom); }

  //: Rect - Rect operator.
  constexpr Rectf operator-(const Rectf &r) const noexcept
  { return Rectf(left - r.left, top - r.top, right - r.right, bottom - r.bottom); }

  //: Rect + Vector2i operator.
  constexpr Rectf operator+(const Vector2f &p) const noexcept
  { return Rectf(left + p.x, top + p.y, right + p.x, bottom + p.y); }

  //: Rect - Vector2i operator.
  constexpr Rectf operator-(const Vector2f &p) const noexcept
  { return Rectf(left - p.x, top - p.y, right - p.x, bottom - p.y); }

  //: Rect == Rect operator.
  constexpr bool operator==(const Rectf &r) const noexcept
  { return (left == r.left && top == r.top && right == r.right && bottom == r.bottom); }

  //: Rect != Rect operator.
  constexpr bool operator!=(const Rectf &r) const noexcept
  { return (left != r.left || top != r.top || right != r.right || bottom != r.bottom); }

  //! Attributes:
public:
  //: X1-coordinate.
  float left;

  //: Y1-coordinate.
  float top;

  //: X2-coordinate.
  float right;

  //: Y2-coordinate.
  float bottom;

  //: Returns the width of the rectangle.
  constexpr float get_width() const noexcept { return right - left; }

  //: Returns the height of the rectangle.
  constexpr float get_height() const noexcept { return bottom - top; }

  //: Returns the size of the rectangle.
  constexpr Sizef get_size() const noexcept { return Sizef(right - left, bottom - top); }

  //: Returns true if point is inside the rectangle.
  constexpr bool is_inside(const Vector2f &p) const noexcept { return (p.x >= left && p.y >= top && p.x <= right && p.y <= bottom); }

  //: Returns true if rectangle passed is overlapping or inside this rectangle.
  constexpr bool is_overlapped(const Rectf &r) const noexcept
  {
    return (r.left < right && r.right > left && r.top < bottom && r.bottom > top);
  }

  //! Operations:
public:
  //: Sets the size of the rectangle, maintaining top/left position.
  constexpr void set_size(const Size &size) noexcept
  {
    right  = left + static_cast<float>(size.width);
    bottom = top  + static_cast<float>(size.height);
  }

  //: Calculates and returns the union of two rectangles.
  constexpr Rectf calc_union(const Rectf &rect) noexcept
  {
    Rectf result;
    if (left   > rect.left)   result.left   = left;   else result.left   = rect.left;
    if (right  < rect.right)  result.right  = right;  else result.right  = rect.right;
    if (top    > rect.top)    result.top    = top;    else result.top    = rect.top;
    if (bottom < rect.bottom) result.bottom = bottom; else result.bottom = rect.bottom;
    return result;
  }

  constexpr bool is_normal() const noexcept
  {
    return left <= right && top <= bottom;
  }

  //: Normalize rectangle. Ensures that left<right and top<bottom.
  constexpr void normalize() noexcept
  {
    if (left > right)
    {
      float temp = right;
      right = left;
      left = temp;
    }

    if (top > bottom)
    {
      float temp = bottom;
      bottom = top;
      top = temp;
    }
  }

  constexpr Vector2f get_center() const noexcept {
    return Vector2f((left + right) / 2.0f,
                    (top + bottom) / 2.0f);
  }

  // Moves each edge f away from the center, thus width = old_width + 2*f
  constexpr Rectf grow(float f) const noexcept {
    return Rectf(left   - f,
                 top    - f,
                 right  + f,
                 bottom + f);
  }

  constexpr Rectf grow(float x, float y) const noexcept {
    return Rectf(left   - x,
                 top    - y,
                 right  + x,
                 bottom + y);
  }

  // Construct a rectangle large enough
  constexpr Rectf grow(const Rectf& rect) const noexcept {
    return Rectf(std::min(left, rect.left),
                 std::min(top, rect.top),
                 std::max(right, rect.right),
                 std::max(bottom, rect.bottom));
  }

  float get_diagonal() const
  {
    return std::sqrt((get_width() * get_width()) + (get_height() * get_height()));
  }

  //: Returns true if point is inside the rectangle.
  constexpr bool contains(const Vector2f &p) const noexcept { return (p.x >= left && p.y >= top && p.x <= right && p.y <= bottom); }

  //: Check if rect is inside this
  constexpr bool contains(const Rectf& rect) const noexcept
  {
    return
      left   <= rect.left  &&
      right  >= rect.right &&
      top    <= rect.top   &&
      bottom >= rect.bottom;
  }

  constexpr Rectf clip_to(const Rectf& cliprect) const noexcept
  {
    return Rectf(std::max(left,   cliprect.left),
                 std::max(top,    cliprect.top),
                 std::min(right,  cliprect.right),
                 std::min(bottom, cliprect.bottom));
  }
};

inline constexpr Rect::Rect(const Rectf& rect) noexcept
  : left(static_cast<int>(rect.left)),
    top(static_cast<int>(rect.top)),
    right(static_cast<int>(rect.right)),
    bottom(static_cast<int>(rect.bottom))
{}

std::ostream& operator<<(std::ostream& s, const Rect& rect);
std::ostream& operator<<(std::ostream& s, const Rectf& rect);


} // namespace pingus
#endif

// EOF
