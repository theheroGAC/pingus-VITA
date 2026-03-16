// src/engine/input/core_driver.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2007 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "engine/input/core_driver.hpp"

#include <algorithm>

#include "engine/display/display.hpp"
#include "engine/input/manager.hpp"
#include "util/log.hpp"

namespace pingus::input {

class AxisPointer : public Pointer
{
private:
  std::unique_ptr<Axis> x_axis;
  std::unique_ptr<Axis> y_axis;
  std::unique_ptr<Button> speed_button;
  float speed;

public:
  AxisPointer(Control* parent_) :
    Pointer(parent_),
    x_axis(), y_axis(), speed_button(),
    speed(400.0f)
  {
  }

  ~AxisPointer()
  {
  }

  void setup(std::unique_ptr<Axis> x, std::unique_ptr<Axis> y, std::unique_ptr<Button> s = {})
  {
    x_axis = std::move(x);
    y_axis = std::move(y);
    speed_button = std::move(s);
  }

  void update(Control* )
  {
    //log_error("event");
  }

  void update(float delta)
  {
    if (!x_axis || !y_axis) return;
    x_axis->update(delta);
    y_axis->update(delta);
    if (speed_button) speed_button->update(delta);

    Vector2f new_pos = pos;
    float c_speed = speed;

    if (speed_button && speed_button->get_state() == BUTTON_PRESSED)
    {
      c_speed *= 5.0f;
    }

    new_pos.x += x_axis->get_pos() * c_speed * delta;
    new_pos.y += y_axis->get_pos() * c_speed * delta;

    // FIXME: shouldn't depend on Display
    new_pos.x = std::clamp(new_pos.x, 0.0f, static_cast<float>(Display::get_width()));
    new_pos.y = std::clamp(new_pos.y, 0.0f, static_cast<float>(Display::get_height()));

    if (new_pos != pos)
    {
      pos = new_pos;
      notify_parent();
    }
  }

private:
  AxisPointer(const AxisPointer&);
  AxisPointer & operator=(const AxisPointer&);
};

class AxisScroller : public Scroller
{
private:
  std::unique_ptr<Axis> x_axis;
  std::unique_ptr<Axis> y_axis;
  std::unique_ptr<Button> speed_button;
  float speed;

public:
  AxisScroller(Control* parent_) :
    Scroller(parent_),
    x_axis(), y_axis(), speed_button(),
    speed(800.0f)
  {
  }

  ~AxisScroller()
  {
  }

  void setup(std::unique_ptr<Axis> x, std::unique_ptr<Axis> y, std::unique_ptr<Button> s = {})
  {
    x_axis = std::move(x);
    y_axis = std::move(y);
    speed_button = std::move(s);
  }

  void update(Control* )
  {
    //log_error("event");
  }

  void update(float delta_)
  {
    if (!x_axis || !y_axis) return;
    x_axis->update(delta_);
    y_axis->update(delta_);

    if (speed_button) speed_button->update(delta_);

    float    c_speed = speed;

    if (speed_button && speed_button->get_state() == BUTTON_PRESSED)
    {
      c_speed *= 5.0f;
    }

    this->delta.x = -x_axis->get_pos() * c_speed * delta_;
    this->delta.y = y_axis->get_pos() * c_speed * delta_;

    notify_parent();
  }

private:
  AxisScroller(const AxisScroller&);
  AxisScroller & operator=(const AxisScroller&);
};

class ButtonScroller : public Scroller
{
private:
  std::unique_ptr<Button> up;
  std::unique_ptr<Button> down;
  std::unique_ptr<Button> left;
  std::unique_ptr<Button> right;
  float speed;

public:
  ButtonScroller(Control* parent_) :
    Scroller(parent_),
    up(), down(), left(), right(),
    speed(800.0f)
  {
  }

  ~ButtonScroller()
  {
  }

  void setup(std::unique_ptr<Button> up_,
             std::unique_ptr<Button> down_,
             std::unique_ptr<Button> left_,
             std::unique_ptr<Button> right_)
  {
    up    = std::move(up_);
    down  = std::move(down_);
    left  = std::move(left_);
    right = std::move(right_);
  }

  void update(Control* )
  {
  }

  void update(float delta_t)
  {
    // Guard against null buttons: create_button() can return nullptr when
    // the underlying driver can't find the requested joystick device.
    // Calling a virtual function through a null pointer causes a PC=0 ISI
    // crash on Wii (null vtable lookup). Skip the update entirely if any
    // button failed to initialise.
    if (!up || !down || !left || !right)
      return;

    up->update(delta_t);
    down->update(delta_t);
    left->update(delta_t);
    right->update(delta_t);

    delta.x = delta.y = 0.0f;

    if (left->get_state() == BUTTON_PRESSED)
      delta.x += speed * delta_t;

    if (right->get_state() == BUTTON_PRESSED)
      delta.x += -speed * delta_t;

    if (up->get_state() == BUTTON_PRESSED)
      delta.y += speed * delta_t;

    if (down->get_state() == BUTTON_PRESSED)
      delta.y += -speed * delta_t;

    notify_parent();
  }

private:
  ButtonScroller(const ButtonScroller&);
  ButtonScroller & operator=(const ButtonScroller&);
};

std::unique_ptr<Button>
CoreDriver::create_button(const FileReader& /*reader*/, Control* /*parent*/)
{
  return nullptr;
}

std::unique_ptr<Axis>
CoreDriver::create_axis(const FileReader& /*reader*/, Control* /*parent*/)
{
  return nullptr;
}

std::unique_ptr<Scroller>
CoreDriver::create_scroller(const FileReader& reader, Control* parent)
{
  if (reader.get_name() == "core:axis-scroller")
  {
    auto axis = std::make_unique<AxisScroller>(parent);

    FileReader x_reader;
    if (!reader.read_section("x-axis", x_reader))
    {
      log_error("CoreDriver: Couldn't find x-axis");
      return nullptr;
    }

    FileReader y_reader;
    if (!reader.read_section("y-axis", y_reader))
    {
      log_error("CoreDriver: Couldn't find y-axis");
      return nullptr;
    }

    auto x_axis = manager->create_axis(x_reader.get_sections().front(), axis.get());
    auto y_axis = manager->create_axis(y_reader.get_sections().front(), axis.get());

    std::unique_ptr<Button> button;
    FileReader button_reader;
    if (reader.read_section("button", button_reader))
    {
      button = manager->create_button(button_reader.get_sections().front(), axis.get());
    }

    if (x_axis && y_axis)
    {
      axis->setup(std::move(x_axis), std::move(y_axis), std::move(button));
      return axis;
    }
    else
    {
      return nullptr;
    }
  }
  else if (reader.get_name() == "core:button-scroller")
  {
    auto scroller = std::make_unique<ButtonScroller>(parent);

    FileReader left_reader;
    if (!reader.read_section("left", left_reader))
    {
      log_error("CoreDriver: core:button-scroller: Couldn't find 'left'");
      return nullptr;
    }

    FileReader right_reader;
    if (!reader.read_section("right", right_reader))
    {
      log_error("CoreDriver: core:button-scroller: Couldn't find 'right'");
      return nullptr;
    }

    FileReader up_reader;
    if (!reader.read_section("up", up_reader))
    {
      log_error("CoreDriver: core:button-scroller: Couldn't find 'up'");
      return nullptr;
    }

    FileReader down_reader;
    if (!reader.read_section("down", down_reader))
    {
      log_error("CoreDriver: core:button-scroller: Couldn't find 'down'");
      return nullptr;
    }

    // FIXME: Add more error checking
    auto up_button    = manager->create_button(up_reader.get_sections().front(),    scroller.get());
    auto down_button  = manager->create_button(down_reader.get_sections().front(),  scroller.get());
    auto left_button  = manager->create_button(left_reader.get_sections().front(),  scroller.get());
    auto right_button = manager->create_button(right_reader.get_sections().front(), scroller.get());

    scroller->setup(std::move(up_button),
                    std::move(down_button),
                    std::move(left_button),
                    std::move(right_button));
    return scroller;
  }
  else
  {
    return nullptr;
  }
}

std::unique_ptr<Pointer>
CoreDriver::create_pointer(const FileReader& reader, Control* parent)
{
  if (reader.get_name() == "core:axis-pointer")
  {
    auto axis = std::make_unique<AxisPointer>(parent);

    FileReader x_reader;
    if (!reader.read_section("x-axis", x_reader))
    {
      log_error("CoreDriver: Couldn't find x-axis");
      return nullptr;
    }

    FileReader y_reader;
    if (!reader.read_section("y-axis", y_reader))
    {
      log_error("CoreDriver: Couldn't find y-axis");
      return nullptr;
    }

    auto x_axis = manager->create_axis(x_reader.get_sections().front(), axis.get());
    auto y_axis = manager->create_axis(y_reader.get_sections().front(), axis.get());

    std::unique_ptr<Button> button;
    FileReader button_reader;
    if (reader.read_section("button", button_reader))
    {
      button = manager->create_button(button_reader.get_sections().front(), axis.get());
    }

    if (x_axis && y_axis)
    {
      axis->setup(std::move(x_axis), std::move(y_axis), std::move(button));
      return axis;
    }
    else
    {
      return nullptr;
    }
  }
  else
  {
    return nullptr;
  }
}

std::unique_ptr<Keyboard>
CoreDriver::create_keyboard(const FileReader& /*reader*/, Control* /*parent*/)
{
  return nullptr;
}

} // namespace pingus::input

// EOF
