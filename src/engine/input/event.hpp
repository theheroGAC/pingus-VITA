// src/engine/input/event.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2000 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_INPUT_EVENT_HPP
#define HEADER_PINGUS_ENGINE_INPUT_EVENT_HPP

#include <string>
#include <vector>

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_events.h>

namespace pingus::input {

enum EventType { BUTTON_EVENT_TYPE,
                 POINTER_EVENT_TYPE,
                 AXIS_EVENT_TYPE,
                 SCROLLER_EVENT_TYPE,
                 KEYBOARD_EVENT_TYPE };

enum EventName {
  // Buttons
  PRIMARY_BUTTON,
  SECONDARY_BUTTON,
  PAUSE_BUTTON,
  FAST_FORWARD_BUTTON,
  SINGLE_STEP_BUTTON,
  ARMAGEDDON_BUTTON,
  ESCAPE_BUTTON,

  ACTION_1_BUTTON,
  ACTION_2_BUTTON,
  ACTION_3_BUTTON,
  ACTION_4_BUTTON,
  ACTION_5_BUTTON,
  ACTION_6_BUTTON,
  ACTION_7_BUTTON,
  ACTION_8_BUTTON,
  ACTION_9_BUTTON,
  ACTION_10_BUTTON,

  ACTION_UP_BUTTON,
  ACTION_DOWN_BUTTON,

  // Pointer
  STANDARD_POINTER,

  // Scroller
  STANDARD_SCROLLER,

  STANDARD_KEYBOARD,

  // Axis
  ACTION_AXIS
};

enum ButtonState { BUTTON_RELEASED, BUTTON_PRESSED };

struct ButtonEvent
{
  EventName   name;
  ButtonState state;
};

struct PointerEvent
{
  EventName name;
  float x;
  float y;
};

struct AxisEvent
{
  EventName name;
  float     dir;
};

struct ScrollEvent
{
  EventName name;
  float x_delta;
  float y_delta;
};

struct KeyboardEvent
{
  bool       state;
  SDL_Keysym keysym;
  uint32_t   unicode;
};

struct Event
{
  EventType type;

  union {
    ButtonEvent   button;
    PointerEvent  pointer;
    AxisEvent     axis;
    ScrollEvent   scroll;
    KeyboardEvent keyboard;
  };
};

typedef std::vector<Event> EventLst;

inline Event makeButtonEvent(EventName name, ButtonState state)
{
  Event event;

  event.type  = BUTTON_EVENT_TYPE;
  event.button.name  = name;
  event.button.state = state;

  return event;
}

inline Event makePointerEvent(EventName name, float x, float y)
{
  Event event;

  event.type = POINTER_EVENT_TYPE;
  event.pointer.name = name;
  event.pointer.x    = x;
  event.pointer.y    = y;

  return event;
}

inline Event makeAxisEvent(EventName name, float dir)
{
  Event event;

  event.type = AXIS_EVENT_TYPE;
  event.axis.dir  = dir;
  event.axis.name = name;

  return event;
}

inline Event makeScrollerEvent(EventName name, float x_delta, float y_delta)
{
  Event event;

  event.type    = SCROLLER_EVENT_TYPE;
  event.scroll.name    = name;
  event.scroll.x_delta = x_delta;
  event.scroll.y_delta = y_delta;

  return event;
}

// Build a KeyboardEvent from SDL_KEYDOWN/SDL_KEYUP.
// unicode is 0 here; filled in separately from SDL_TEXTINPUT.
inline Event makeKeyboardEvent(const SDL_KeyboardEvent& ev)
{
  Event event;

  event.type             = KEYBOARD_EVENT_TYPE;
  event.keyboard.state   = (ev.state == SDL_PRESSED);
  event.keyboard.keysym  = ev.keysym;
  event.keyboard.unicode = 0;

  return event;
}

// Build a keyboard event carrying only a Unicode codepoint (from SDL_TEXTINPUT).
inline Event makeTextInputEvent(uint32_t codepoint)
{
  Event event;

  event.type             = KEYBOARD_EVENT_TYPE;
  event.keyboard.state   = true;
  event.keyboard.keysym  = {};
  event.keyboard.unicode = codepoint;

  return event;
}

} // namespace pingus::input

#endif

// EOF
