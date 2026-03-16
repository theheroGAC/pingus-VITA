// src/engine/input/control.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2007 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_INPUT_CONTROL_HPP
#define HEADER_PINGUS_ENGINE_INPUT_CONTROL_HPP

#include <algorithm>
#include <vector>
#include <memory>

#include "engine/input/controller.hpp"
#include "engine/input/event.hpp"
#include "math/math.hpp"
#include "math/vector2f.hpp"
#include "util/log.hpp"

namespace pingus::input {

class Control
{
private:
  Control* parent;

public:
  Control(Control* parent_)
    : parent(parent_)
  {}

  virtual ~Control() {
  }

  virtual void notify_parent()
  {
    if (parent)
    {
      parent->update(this);
    }
    else
    {
      log_error("parent missing!");
    }
  }

  virtual void update(float /*delta*/) {
  }

  virtual void update(Control* /*ctrl*/) {
    log_warn("Control:update() not handled");
  }

private:
  Control(const Control&);
  Control & operator=(const Control&);
};

class Button : public Control
{
protected:
  ButtonState state;

public:
  Button(Control* parent_) :
    Control(parent_),
    state(BUTTON_RELEASED)
  {}

  bool get_state() const { return state; }

  virtual void set_state(ButtonState new_state)
  {
    if (new_state != state)
    {
      state = new_state;
      notify_parent();
    }
  }
};

class ButtonGroup : public Button
{
private:
  std::vector<std::unique_ptr<Button>> buttons;

public:
  ButtonGroup(Control* parent_) :
    Button(parent_),
    buttons()
  {}

  ~ButtonGroup()
  {
  }

  void add_button(std::unique_ptr<Button> button_) {
    buttons.push_back(std::move(button_));
  }

  void update(float delta) {
    for(auto i = buttons.begin(); i != buttons.end(); ++i)
      (*i)->update(delta);
  }

  virtual void update(Control* /*ctrl*/)
  {
    ButtonState new_state = BUTTON_RELEASED;

    for(auto i = buttons.begin();
        i != buttons.end(); ++i)
    {
      if ((*i)->get_state() == BUTTON_PRESSED)
        new_state = BUTTON_PRESSED;
    }

    if (new_state != state)
    {
      state = new_state;
      notify_parent();
    }
  }
};

class ControllerButton : public ButtonGroup
{
private:
  Controller* controller;
  int id;

public:
  ControllerButton(Controller* controller_, int id_)
    : ButtonGroup(nullptr),
      controller(controller_),
      id(id_)
  {}

  virtual void notify_parent() {
    controller->add_button_event(id, state);
  }

private:
  ControllerButton(const ControllerButton&);
  ControllerButton & operator=(const ControllerButton&);
};

class Axis : public Control
{
protected:
  float pos;
  float dead_zone;
  bool  invert;

public:
  Axis(Control* parent_) :
    Control(parent_),
    pos(0.0f),
    dead_zone(0.2f),
    invert(false)
  {}

  float get_pos() const { return pos; }

  virtual void set_state(float new_pos) {
    if (invert)
      new_pos = -new_pos;

    if (std::abs(new_pos) < dead_zone)
      new_pos = 0.0f;

    if (new_pos != pos)
    {
      pos = new_pos;
      notify_parent();
    }
  }
};

class Pointer : public Control
{
protected:
  Vector2f pos;

public:
  Pointer(Control* parent_) :
    Control(parent_),
    pos()
  {}

  Vector2f get_pos() const { return pos; }

  void set_pos(const Vector2f& new_pos) {
    if (pos != new_pos)
    {
      pos = new_pos;
      notify_parent();
    }
  }
};

class Scroller : public Control
{
protected:
  Vector2f delta;

public:
  Scroller(Control* parent_) :
    Control(parent_),
    delta(0.0f, 0.0f)
  {}

  Vector2f get_delta() const { return delta; }

  void set_delta(const Vector2f& new_delta) {
    if (delta != new_delta)
    {
      delta = new_delta;
      notify_parent();
    }
  }
};

class AxisGroup : public Axis {
private:
  std::vector<std::unique_ptr<Axis>> axes;

public:
  AxisGroup(Control* parent_) :
    Axis(parent_),
    axes()
  {}

  ~AxisGroup()
  {
  }

  void add_axis(std::unique_ptr<Axis> axis) {
    axes.push_back(std::move(axis));
  }

  void update(float delta) {
    for(auto i = axes.begin(); i != axes.end(); ++i)
      (*i)->update(delta);
  }

  void update(Control* /*ctrl*/)
  {
    float new_pos = 0;

    for(auto i = axes.begin(); i != axes.end(); ++i)
    {
      new_pos += (*i)->get_pos();
    }

    new_pos = std::clamp(new_pos, -1.0f, 1.0f);

    set_state(new_pos);
  }
};

class ControllerAxis : public AxisGroup
{
private:
  Controller* controller;
  int id;

public:
  ControllerAxis(Controller* controller_, int id_)
    : AxisGroup(nullptr),
      controller(controller_),
      id(id_)
  {}

  virtual void notify_parent() {
    controller->add_axis_event(id, pos);
  }

private:
  ControllerAxis(const ControllerAxis&);
  ControllerAxis & operator=(const ControllerAxis&);
};

class PointerGroup : public Pointer
{
private:
  std::vector<std::unique_ptr<Pointer>> pointer;

public:
  PointerGroup(Control* parent_) :
    Pointer(parent_),
    pointer()
  {}

  ~PointerGroup()
  {
  }

  void update(Control* p) {
    Pointer* pointer_ = dynamic_cast<Pointer*>(p);
    assert(pointer_);
    Vector2f new_pos = pointer_->get_pos();
    if (new_pos != pos)
    {
      pos = new_pos;
      notify_parent();
    }
  }

  void update(float delta) {
    for(auto i = pointer.begin(); i != pointer.end(); ++i)
      (*i)->update(delta);
  }

  void add_pointer(std::unique_ptr<Pointer> p) {
    pointer.push_back(std::move(p));
  }
};

class ControllerPointer : public PointerGroup
{
private:
  Controller* controller;
  int id;

public:
  ControllerPointer(Controller* controller_, int id_)
    : PointerGroup(nullptr),
      controller(controller_),
      id(id_)
  {}

  virtual void notify_parent() {
    controller->add_pointer_event(id, pos.x, pos.y);
  }

private:
  ControllerPointer(const ControllerPointer&);
  ControllerPointer & operator=(const ControllerPointer&);
};

class ScrollerGroup : public Scroller
{
private:
  std::vector<std::unique_ptr<Scroller>> scrollers;

public:
  ScrollerGroup(Control* parent_) :
    Scroller(parent_),
    scrollers()
  {}

  ~ScrollerGroup()
  {
  }

  void update(float delta_) {
    for(auto i = scrollers.begin(); i != scrollers.end(); ++i)
      (*i)->update(delta_);
  }

  void update(Control* p) {
    Scroller* scroller = dynamic_cast<Scroller*>(p);
    assert(scroller);
    delta = scroller->get_delta();
    notify_parent();
  }

  void add_scroller(std::unique_ptr<Scroller> p) {
    scrollers.push_back(std::move(p));
  }

private:
  ScrollerGroup(const ScrollerGroup&);
  ScrollerGroup & operator=(const ScrollerGroup&);
};

class ControllerScroller : public ScrollerGroup
{
private:
  Controller* controller;
  int id;

public:
  ControllerScroller(Controller* controller_, int id_)
    : ScrollerGroup(nullptr),
      controller(controller_),
      id(id_)
  {}

  virtual void notify_parent() {
    controller->add_scroller_event(id, delta.x, delta.y);
  }

private:
  ControllerScroller(const ControllerScroller&);
  ControllerScroller & operator=(const ControllerScroller&);
};

class Keyboard : public Control
{
protected:
  SDL_KeyboardEvent m_ev;
  uint32_t          m_unicode; // UTF-32 codepoint from SDL_TEXTINPUT, or 0

public:
  Keyboard(Control* parent_) :
    Control(parent_),
    m_ev(),
    m_unicode(0)
  {}

  // Called for SDL_KEYDOWN / SDL_KEYUP events
  void send_char(const SDL_KeyboardEvent& ev)
  {
    m_ev      = ev;
    m_unicode = 0;
    notify_parent();
  }

  // Called for SDL_TEXTINPUT events; carries the UTF-32 codepoint
  void send_text(uint32_t codepoint)
  {
    m_ev      = {};       // no physical key info
    m_unicode = codepoint;
    notify_parent();
  }

  SDL_KeyboardEvent get_ev()      { return m_ev; }
  uint32_t          get_unicode() { return m_unicode; }

private:
  Keyboard(const Keyboard&);
  Keyboard & operator=(const Keyboard&);
};

class KeyboardGroup : public Keyboard
{
private:
  std::vector<std::unique_ptr<Keyboard>> keyboards;

public:
  KeyboardGroup(Control* parent_) :
    Keyboard(parent_),
    keyboards()
  {}

  ~KeyboardGroup()
  {
  }

  void update(float /*delta*/) {
  }

  void update(Control* p) {
    Keyboard* kb = dynamic_cast<Keyboard*>(p);
    m_ev      = kb->get_ev();
    m_unicode = kb->get_unicode();
    notify_parent();
  }

  void add_keyboard(std::unique_ptr<Keyboard> keyboard)
  {
    keyboards.push_back(std::move(keyboard));
  }
};

class ControllerKeyboard : public KeyboardGroup
{
private:
  Controller* controller;
  int id;

public:
  ControllerKeyboard(Controller* controller_, int id_) :
    KeyboardGroup(nullptr),
    controller(controller_),
    id(id_)
  {}

  virtual void notify_parent() {
    controller->add_keyboard_event(m_ev, m_unicode);
  }

private:
  ControllerKeyboard(const ControllerKeyboard&);
  ControllerKeyboard & operator=(const ControllerKeyboard&);
};

} // namespace pingus::input

#endif

// EOF
