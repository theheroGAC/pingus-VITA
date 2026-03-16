// src/engine/input/sdl_driver.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2007 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_INPUT_SDL_DRIVER_HPP
#define HEADER_PINGUS_ENGINE_INPUT_SDL_DRIVER_HPP

#include <map>
#include <vector>
#include <memory>

#include <SDL.h>

#include "engine/input/control.hpp"
#include "engine/input/driver.hpp"

class FileReader;

namespace pingus::input {

class SDLDriver : public Driver
{
private:
  struct JoystickButtonBinding {
    Button* binding;

    int device;
    int button;
  };

  struct JoystickHatBinding {
    Button* binding;

    int device;
    int hat;
    int dir; // SDL_HAT_UP, SDL_HAT_DOWN, etc.
  };

  struct JoystickAxisBinding {
    Axis* binding;

    int  device;
    int  axis;
  };

  struct MouseButtonBinding {
    Button* binding;

    int button;
  };

  struct KeyboardButtonBinding {
    Button* binding;

    SDL_Keycode key;
  };

  struct ScrollerBinding {
    Scroller* binding;
  };

  struct PointerBinding {
    Pointer* binding;
  };

  std::vector<ScrollerBinding>       scroller_bindings;
  std::vector<PointerBinding>        pointer_bindings;
  std::vector<KeyboardButtonBinding> keyboard_button_bindings;
  std::vector<MouseButtonBinding>    mouse_button_bindings;
  std::vector<JoystickButtonBinding> joystick_button_bindings;
  std::vector<JoystickHatBinding>    joystick_hat_bindings;
  std::vector<JoystickAxisBinding>   joystick_axis_bindings;
  Keyboard* keyboard_binding;

  typedef std::map<std::string, SDL_Keycode> String2Key;
  String2Key string2key;

  typedef std::map<int, SDL_Joystick*> JoystickHandles;
  JoystickHandles joystick_handles;

#ifdef __WII__
  SDL_Event custom_queue[32];
  int queue_head;
  int queue_tail;
  uint8_t last_hat;
#endif

  // Virtual Mouse State for Joystick Emulation
  float virtual_mouse_x;
  float virtual_mouse_y;
  float joy_axis_x;
  float joy_axis_y;

public:
  SDLDriver();
  ~SDLDriver();

  std::unique_ptr<Button>   create_button  (const FileReader& reader, Control* parent);
  std::unique_ptr<Axis>     create_axis    (const FileReader& reader, Control* parent);
  std::unique_ptr<Scroller> create_scroller(const FileReader& reader, Control* parent);
  std::unique_ptr<Pointer>  create_pointer (const FileReader& reader, Control* parent);
  std::unique_ptr<Keyboard> create_keyboard(const FileReader& reader, Control* parent);

  void update(float delta);
  std::string get_name() const { return "sdl"; }

private:
  bool open_joystick(int device);

private:
  SDLDriver(const SDLDriver&);
  SDLDriver & operator=(const SDLDriver&);
};

} // namespace pingus::input

#endif

// EOF
