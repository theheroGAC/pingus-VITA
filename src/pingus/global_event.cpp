// src/pingus/global_event.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2000 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/global_event.hpp"

#include "engine/display/screenshot.hpp"
#include "engine/screen/screen_manager.hpp"
#include "pingus/config_manager.hpp"
#include "pingus/globals.hpp"
#include "pingus/screens/addon_menu.hpp"
#include "pingus/screens/option_menu.hpp"
#include "util/log.hpp"

namespace pingus {


GlobalEvent global_event;

GlobalEvent::GlobalEvent ()
{
}

void
GlobalEvent::on_button_press(const SDL_KeyboardEvent& event)
{
  const Uint8* keystate = SDL_GetKeyboardState(NULL);

  switch (event.keysym.sym)
  {
    case SDLK_F10:
      config_manager.set_print_fps(!config_manager.get_print_fps());
      break;

    case SDLK_RETURN:
      if (keystate[SDL_SCANCODE_LALT] || keystate[SDL_SCANCODE_RALT])
      {
        config_manager.set_fullscreen(!config_manager.get_fullscreen());
      }
      break;

    case SDLK_TAB: // unlock mouse grab if Alt-Tab is pressed to allow the user to change windows
      if (config_manager.get_mouse_grab())
      {
        if (keystate[SDL_SCANCODE_LALT] || keystate[SDL_SCANCODE_RALT])
        {
          // FIXME: should suspend the grab till the user clicks the
          // window again, not completely disable it
          config_manager.set_mouse_grab(false);
        }
      }
      break;

    case SDLK_F11:
      config_manager.set_fullscreen(!config_manager.get_fullscreen());
      break;

    case SDLK_F5:
      if (!dynamic_cast<OptionMenu*>(ScreenManager::instance()->get_current_screen().get()))
        ScreenManager::instance()->push_screen(std::make_shared<OptionMenu>());
      break;

    case SDLK_o:
      if (keystate[SDL_SCANCODE_LCTRL] || keystate[SDL_SCANCODE_RCTRL])
      {
        if (!dynamic_cast<OptionMenu*>(ScreenManager::instance()->get_current_screen().get()))
          ScreenManager::instance()->push_screen(std::make_shared<OptionMenu>());
      }
      break;

    case SDLK_F6:
      if (globals::developer_mode)
      {
        if (!dynamic_cast<AddOnMenu*>(ScreenManager::instance()->get_current_screen().get()))
          ScreenManager::instance()->push_screen(std::make_shared<AddOnMenu>());
      }
      break;

    case SDLK_F12:
      {
        Screenshot::make_screenshot();
      }
      break;

    case SDLK_c:
      if (globals::developer_mode)
        globals::draw_collision_map = !globals::draw_collision_map;
      break;

    case SDLK_k:
      if (globals::developer_mode)
        log_info("Low level screen clear triggered (no-op in SDL2 build)");
      break;

    case SDLK_m:
      if (keystate[SDL_SCANCODE_LCTRL] || keystate[SDL_SCANCODE_RCTRL])
      {
        log_info("Developer Mode: {}", globals::developer_mode);
        globals::developer_mode = !globals::developer_mode;
      }
      break;

    case SDLK_g:
      if (keystate[SDL_SCANCODE_LCTRL] || keystate[SDL_SCANCODE_RCTRL])
      {
        config_manager.set_mouse_grab(!config_manager.get_mouse_grab());
      }
      break;

    case SDLK_KP_PLUS:
      globals::game_speed -= 5;
      if (globals::game_speed < 5)
        globals::game_speed = 5;
      break;

    case SDLK_KP_MINUS:
      globals::game_speed += 5;
      break;

    case SDLK_KP_ENTER:
      globals::game_speed = 20;
      break;

    default:
      // console << "GlobalEvent: Unknown key pressed: " << key.id;
      break;
  }
}

void
GlobalEvent::on_button_release(const SDL_KeyboardEvent& /*event*/)
{
}


} // namespace pingus

// EOF
