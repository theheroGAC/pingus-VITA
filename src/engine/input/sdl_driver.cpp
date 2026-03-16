// src/engine/input/sdl_driver.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2007 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "engine/input/sdl_driver.hpp"

#ifdef __WII__
#include <wiiuse/wpad.h>
#endif

#include <cmath>
#include "engine/display/display.hpp"
#include "engine/screen/screen_manager.hpp"
#include "pingus/global_event.hpp"

namespace pingus::input {

SDLDriver::SDLDriver() :
  scroller_bindings(),
  pointer_bindings(),
  keyboard_button_bindings(),
  mouse_button_bindings(),
  joystick_button_bindings(),
  joystick_hat_bindings(),
  joystick_axis_bindings(),
  keyboard_binding(nullptr),
  string2key(),
  joystick_handles(),
#ifdef __WII__
  queue_head(0),
  queue_tail(0),
  last_hat(0), // SDL_HAT_CENTERED is 0
#endif
  virtual_mouse_x(Display::get_width() / 2.0f),
  virtual_mouse_y(Display::get_height() / 2.0f),
  joy_axis_x(0.0f),
  joy_axis_y(0.0f)
{
  for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
  {
    SDL_Keycode kc = SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(i));
    if (kc == SDLK_UNKNOWN)
      continue;
    const char* key_name = SDL_GetKeyName(kc);
    if (key_name && key_name[0] != '\0')
    {
      string2key[key_name] = kc;
      log_debug("Key: '{}'", key_name);
    }
  }
}

SDLDriver::~SDLDriver()
{

}

std::unique_ptr<Keyboard>
SDLDriver::create_keyboard(const FileReader& /*reader*/, Control* parent)
{
  auto keyboard = std::make_unique<Keyboard>(parent);
  keyboard_binding = keyboard.get();
  return keyboard;
}

std::unique_ptr<Button>
SDLDriver::create_button(const FileReader& reader, Control* parent)
{
  //log_info("SDL: {}", reader.get_name());
  if (reader.get_name() == "sdl:joystick-button")
  {
    JoystickButtonBinding binding;

    reader.read_int("device", binding.device);
    reader.read_int("button", binding.button);

    if (open_joystick(binding.device))
    {
      auto button = std::make_unique<Button>(parent);
      binding.binding = button.get();
      joystick_button_bindings.push_back(binding);

      return button;
    }
    else
    {
      return nullptr;
    }
  }
  else if (reader.get_name() == "sdl:joystick-hat")
  {
    JoystickHatBinding binding;

    reader.read_int("device", binding.device);
    reader.read_int("hat",    binding.hat);

    std::string dir;
    if (reader.read_string("dir", dir))
    {
      if (dir == "up")         binding.dir = SDL_HAT_UP;
      else if (dir == "down")  binding.dir = SDL_HAT_DOWN;
      else if (dir == "left")  binding.dir = SDL_HAT_LEFT;
      else if (dir == "right") binding.dir = SDL_HAT_RIGHT;
      else {
        log_error("unknown hat direction '{}'", dir);
        return nullptr;
      }

      if (open_joystick(binding.device))
      {
        auto button = std::make_unique<Button>(parent);
        binding.binding = button.get();
        joystick_hat_bindings.push_back(binding);
        return button;
      }
    }
    else
    {
      log_error("'dir' missing for joystick-hat binding");
      return nullptr;
    }
    return nullptr;
  }
  else if (reader.get_name() == "sdl:mouse-button")
  {
    auto button = std::make_unique<Button>(parent);

    MouseButtonBinding binding;
    reader.read_int("button", binding.button);
    binding.binding = button.get();
    mouse_button_bindings.push_back(binding);

    return button;
  }
  else if (reader.get_name() == "sdl:keyboard-button")
  {
    std::string key;
    if (reader.read_string("key", key))
    {
      SDL_Keycode kc = SDLK_UNKNOWN;
      String2Key::iterator i = string2key.find(key);
      if (i != string2key.end())
      {
        kc = i->second;
      }
      else
      {
        kc = SDL_GetKeyFromName(key.c_str());
      }

      if (kc != SDLK_UNKNOWN)
      {
        auto button = std::make_unique<Button>(parent);

        KeyboardButtonBinding binding;
        binding.key = kc;
        binding.binding = button.get();
        keyboard_button_bindings.push_back(binding);

        return button;
      }
      else
      {
        log_error("couldn't find keysym for key '{}'", key);
        return nullptr;
      }
    }
    else
    {
      log_error("'key' missing");
      return nullptr;
    }
  }
  else
  {
    return nullptr;
  }
}

std::unique_ptr<Axis>
SDLDriver::create_axis(const FileReader& reader, Control* parent)
{
  if (reader.get_name() == "sdl:joystick-axis")
  {
    JoystickAxisBinding binding;

    reader.read_int("device", binding.device);
    reader.read_int("axis",   binding.axis);

    if (open_joystick(binding.device))
    {
      auto axis = std::make_unique<Axis>(parent);
      binding.binding = axis.get();
      joystick_axis_bindings.push_back(binding);

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

std::unique_ptr<Scroller>
SDLDriver::create_scroller(const FileReader& reader, Control* parent)
{
  if (reader.get_name() == "sdl:mouse-scroller")
  {
    auto scroller = std::make_unique<Scroller>(parent);

    ScrollerBinding binding;
    binding.binding = scroller.get();
    scroller_bindings.push_back(binding);

    return scroller;
  }
  else
  {
    return nullptr;
  }
}

std::unique_ptr<Pointer>
SDLDriver::create_pointer(const FileReader& reader, Control* parent)
{
  if (reader.get_name() == "sdl:mouse-pointer")
  {
    auto pointer = std::make_unique<Pointer>(parent);

    PointerBinding binding;
    binding.binding = pointer.get();
    pointer_bindings.push_back(binding);

    return pointer;
  }
  else
  {
    return nullptr;
  }
}

bool
SDLDriver::open_joystick(int device)
{
#ifdef __WII__
  return true;
#endif

  JoystickHandles::iterator i = joystick_handles.find(device);
  if (i == joystick_handles.end())
  {
    SDL_Joystick* joy = SDL_JoystickOpen(device);
    if (joy)
    {
      joystick_handles[device] = joy;
      return true;
    }
    else
    {
      log_error("couldn't open joystick number {}", device);
      return false;
    }
  }
  else
  {
    return true;
  }
}

void
SDLDriver::update(float delta)
{
  // FIXME: Little hackywacky, better way would be to fetch event
  // loops somewhere else and only forward the relevant SDL_Events to
  // the SDLDriver

  // --- Joystick Mouse Emulation ---
  const float deadzone = 0.15f;
  const float speed = 800.0f; // Pixels per second

  bool joy_moved = false;

  if (std::abs(joy_axis_x) > deadzone) {
    virtual_mouse_x += joy_axis_x * speed * delta;
    joy_moved = true;
  }
  if (std::abs(joy_axis_y) > deadzone) {
    virtual_mouse_y += joy_axis_y * speed * delta;
    joy_moved = true;
  }

  if (joy_moved) {
    Size screen_size = Display::get_size();
    if (virtual_mouse_x < 0) virtual_mouse_x = 0;
    if (virtual_mouse_y < 0) virtual_mouse_y = 0;
    if (virtual_mouse_x > screen_size.width) virtual_mouse_x = screen_size.width;
    if (virtual_mouse_y > screen_size.height) virtual_mouse_y = screen_size.height;

    for(std::vector<PointerBinding>::iterator i = pointer_bindings.begin();
        i != pointer_bindings.end(); ++i)
    {
      i->binding->set_pos(Vector2f(virtual_mouse_x, virtual_mouse_y));
    }
  }
  // --------------------------------

  SDL_Event event;

#ifdef __WII__
  auto poll_custom_event = [&](SDL_Event* ev) -> int {
    // 1. Drain our injected queue
    if (queue_head != queue_tail)
    {
      *ev = custom_queue[queue_head];
      queue_head = (queue_head + 1) % 32;
      return 1;
    }

    // 2. Poll native Wii input
    WPAD_ScanPads();

    // Read IR pointer directly from WPAD; do not use SDL_MOUSEMOTION on Wii
    // as it carries host window coords, not WPAD IR coords.
    {
      // Velocity + invalid-frame counter for extrapolation when ir drops.
      static float ir_vel_x = 0.0f;
      static float ir_vel_y = 0.0f;
      static int   ir_invalid_frames = 0;
      static const int MAX_EXTRAP_FRAMES = 6; // ~100ms at 60fps

      WPADData *wd_ir = WPAD_Data(0);
      if (wd_ir && wd_ir->err == WPAD_ERR_NONE && wd_ir->ir.valid)
      {
        // VRes is 640+128 x 480+128; subtract 64 so edges are reachable
        // before the wiimote reaches its physical pointing extreme (HBC technique).
        float new_x = (wd_ir->ir.x - 64.0f) * Display::LOGICAL_WIDTH  / 640.0f;
        float new_y = (wd_ir->ir.y - 64.0f) * Display::LOGICAL_HEIGHT / 480.0f;
        new_x = std::clamp(new_x, 0.0f, (float)Display::LOGICAL_WIDTH);
        new_y = std::clamp(new_y, 0.0f, (float)Display::LOGICAL_HEIGHT);
        // Record per-frame velocity for extrapolation when ir.valid drops.
        ir_vel_x = new_x - virtual_mouse_x;
        ir_vel_y = new_y - virtual_mouse_y;
        ir_invalid_frames = 0;
        virtual_mouse_x = new_x;
        virtual_mouse_y = new_y;
      }
      else if (ir_invalid_frames < MAX_EXTRAP_FRAMES && (ir_vel_x != 0.0f || ir_vel_y != 0.0f))
      {
        // ir.valid dropped — fast swipe moved the IR dots off the sensor bar.
        // Extrapolate with decaying velocity so a fast fling to the edge
        // actually reaches it rather than freezing short.
        ir_invalid_frames++;
        virtual_mouse_x = std::clamp(virtual_mouse_x + ir_vel_x, 0.0f, (float)Display::LOGICAL_WIDTH);
        virtual_mouse_y = std::clamp(virtual_mouse_y + ir_vel_y, 0.0f, (float)Display::LOGICAL_HEIGHT);
        ir_vel_x *= 0.7f;
        ir_vel_y *= 0.7f;
      }
      else
      {
        ir_invalid_frames++;
        // Cursor stays at last known position.
      }

      for (auto& binding : pointer_bindings)
        binding.binding->set_pos(Vector2f(virtual_mouse_x, virtual_mouse_y));
    }

    uint32_t buttons_down = WPAD_ButtonsDown(0);
    uint32_t buttons_up   = WPAD_ButtonsUp(0);
    uint32_t buttons_held = WPAD_ButtonsHeld(0);

    // Mapping Wii Remote buttons to SDL Joystick Buttons
    struct ButtonMap {
      uint32_t wpad_btn;
      uint8_t sdl_btn;
    };
    ButtonMap bmap[] = {
        {WPAD_BUTTON_A, 0},    {WPAD_BUTTON_B, 1},     {WPAD_BUTTON_1, 2},
        {WPAD_BUTTON_2, 3},    {WPAD_BUTTON_MINUS, 4}, {WPAD_BUTTON_PLUS, 5},
        {WPAD_BUTTON_HOME, 6},
    };

    for (auto &bm : bmap)
    {
      if (buttons_down & bm.wpad_btn)
      {
        SDL_Event e;
        e.type = SDL_JOYBUTTONDOWN;
        e.jbutton.which = 0;
        e.jbutton.button = bm.sdl_btn;
        e.jbutton.state = SDL_PRESSED;
        custom_queue[queue_tail] = e;
        queue_tail = (queue_tail + 1) % 32;
      }
      if (buttons_up & bm.wpad_btn)
      {
        SDL_Event e;
        e.type = SDL_JOYBUTTONUP;
        e.jbutton.which = 0;
        e.jbutton.button = bm.sdl_btn;
        e.jbutton.state = SDL_RELEASED;
        custom_queue[queue_tail] = e;
        queue_tail = (queue_tail + 1) % 32;
      }
    }

    // Handle D-Pad as Hat
    uint8_t hat = SDL_HAT_CENTERED;
    if (buttons_held & WPAD_BUTTON_UP)    hat |= SDL_HAT_UP;
    if (buttons_held & WPAD_BUTTON_DOWN)  hat |= SDL_HAT_DOWN;
    if (buttons_held & WPAD_BUTTON_LEFT)  hat |= SDL_HAT_LEFT;
    if (buttons_held & WPAD_BUTTON_RIGHT) hat |= SDL_HAT_RIGHT;

    if (hat != last_hat) {
      SDL_Event e;
      e.type = SDL_JOYHATMOTION;
      e.jhat.which = 0;
      e.jhat.hat = 0;
      e.jhat.value = hat;
      custom_queue[queue_tail] = e;
      queue_tail = (queue_tail + 1) % 32;
      last_hat = hat;
    }

    // Handle Nunchuk analog stick, guarding against invalid WPAD data
    WPADData *wd = WPAD_Data(0);
    if (wd && wd->err == WPAD_ERR_NONE && wd->exp.type == WPAD_EXP_NUNCHUK)
    {
      static int16_t last_nunchuk_x = 0;
      static int16_t last_nunchuk_y = 0;

      int raw_x = wd->exp.nunchuk.js.pos.x - wd->exp.nunchuk.js.center.x;
      int raw_y = wd->exp.nunchuk.js.pos.y - wd->exp.nunchuk.js.center.y;

      int16_t current_x = (int16_t)(raw_x * 256);
      int16_t current_y = (int16_t)(raw_y * 256);

      const int16_t DEADZONE = 4000;
      if (std::abs(current_x) < DEADZONE) current_x = 0;
      if (std::abs(current_y) < DEADZONE) current_y = 0;

      if (current_x != last_nunchuk_x)
      {
        SDL_Event e;
        e.type = SDL_JOYAXISMOTION;
        e.jaxis.which = 0;
        e.jaxis.axis = 0;
        e.jaxis.value = current_x;
        custom_queue[queue_tail] = e;
        queue_tail = (queue_tail + 1) % 32;
        last_nunchuk_x = current_x;
      }

      if (current_y != last_nunchuk_y)
      {
        SDL_Event e;
        e.type = SDL_JOYAXISMOTION;
        e.jaxis.which = 0;
        e.jaxis.axis = 1;
        e.jaxis.value = -current_y; // Invert Y
        custom_queue[queue_tail] = e;
        queue_tail = (queue_tail + 1) % 32;
        last_nunchuk_y = current_y;
      }
    }

    if (queue_head != queue_tail)
    {
      *ev = custom_queue[queue_head];
      queue_head = (queue_head + 1) % 32;
      return 1;
    }

    return SDL_PollEvent(ev);
  };

  while (poll_custom_event(&event))
#else
  while (SDL_PollEvent(&event))
#endif
  {
    switch(event.type)
    {
      case SDL_QUIT: // FIXME: make this into a GameEvent
        ScreenManager::instance()->pop_all_screens();
        break;

      case SDL_MOUSEMOTION:
      {
#ifdef __WII__
        // On Wii the pointer is updated from WPAD IR above; ignore SDL_MOUSEMOTION.
        break;
#else
        {
          const Size physical = Display::get_physical_size();
          virtual_mouse_x = (float)event.motion.x * Display::LOGICAL_WIDTH  / physical.width;
          virtual_mouse_y = (float)event.motion.y * Display::LOGICAL_HEIGHT / physical.height;
          virtual_mouse_x = std::clamp(virtual_mouse_x, 0.0f, (float)Display::LOGICAL_WIDTH);
          virtual_mouse_y = std::clamp(virtual_mouse_y, 0.0f, (float)Display::LOGICAL_HEIGHT);

          for(std::vector<PointerBinding>::iterator i = pointer_bindings.begin();
              i != pointer_bindings.end(); ++i)
          {
            i->binding->set_pos(Vector2f(virtual_mouse_x, virtual_mouse_y));
          }

          for(std::vector<ScrollerBinding>::iterator i = scroller_bindings.begin();
              i != scroller_bindings.end(); ++i)
          {
            i->binding->set_delta(Vector2f(event.motion.xrel, event.motion.yrel));
          }
        }
        break;
#endif
      }

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        for(std::vector<MouseButtonBinding>::iterator i = mouse_button_bindings.begin();
            i != mouse_button_bindings.end(); ++i)
        {
          if (event.button.button == (*i).button)
          {
            if (event.button.state == SDL_PRESSED)
              (*i).binding->set_state(BUTTON_PRESSED);
            else
              (*i).binding->set_state(BUTTON_RELEASED);
          }
        }
        break;

      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
            event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
          Display::resize(Size(event.window.data1, event.window.data2));
        }
        break;

      case SDL_KEYDOWN:
      case SDL_KEYUP:
        // keyboard events
        if (keyboard_binding)
          keyboard_binding->send_char(event.key);

        // global event hacks
        if (event.key.state == SDL_PRESSED)
          global_event.on_button_press(event.key);
        else
          global_event.on_button_release(event.key);

        // game button events
        for(std::vector<KeyboardButtonBinding>::iterator i = keyboard_button_bindings.begin();
            i != keyboard_button_bindings.end(); ++i)
        {
          if (event.key.keysym.sym == i->key)
          {
            if (event.key.state == SDL_PRESSED)
              i->binding->set_state(BUTTON_PRESSED);
            else
              i->binding->set_state(BUTTON_RELEASED);
          }
        }
        break;

      case SDL_JOYHATMOTION:
        for(std::vector<JoystickHatBinding>::iterator i = joystick_hat_bindings.begin();
            i != joystick_hat_bindings.end(); ++i)
        {
          if (event.jhat.which == i->device &&
              event.jhat.hat   == i->hat)
          {
            bool active = (event.jhat.value & i->dir);
            i->binding->set_state(active ? BUTTON_PRESSED : BUTTON_RELEASED);
          }
        }
        break;

      case SDL_JOYAXISMOTION:
        // Capture Axis 0/1 from ANY device for mouse emulation
        if (event.jaxis.axis == 0) {
            joy_axis_x = static_cast<float>(event.jaxis.value) / 32767.0f;
        } else if (event.jaxis.axis == 1) {
            joy_axis_y = static_cast<float>(event.jaxis.value) / 32767.0f;
        }

        for(std::vector<JoystickAxisBinding>::iterator i = joystick_axis_bindings.begin();
            i != joystick_axis_bindings.end(); ++i)
        {
          if (event.jaxis.which == i->device &&
              event.jaxis.axis  == i->axis)
            i->binding->set_state(static_cast<float>(event.jaxis.value) / 32767.0f);
        }
        break;

      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
        for(std::vector<JoystickButtonBinding>::iterator i = joystick_button_bindings.begin();
            i != joystick_button_bindings.end(); ++i)
        {
          if (event.jbutton.which  == i->device &&
              event.jbutton.button == i->button)
          {
            i->binding->set_state(event.jbutton.state == SDL_PRESSED ? BUTTON_PRESSED : BUTTON_RELEASED);
          }
        }
        break;

      default:
        // FIXME: Do something with other events
        break;
    }
  }
}

} // namespace pingus::input

// EOF
