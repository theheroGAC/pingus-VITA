// src/engine/screen/screen_manager.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2000 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "engine/screen/screen_manager.hpp"

#include <iostream>

#include "engine/display/display.hpp"
#include "engine/display/drawing_context.hpp"
#include "engine/display/framebuffer.hpp"
#include "engine/display/sprite.hpp"
#include "engine/input/manager.hpp"
#include "engine/screen/screen.hpp"
#include "pingus/fps_counter.hpp"
#include "pingus/fonts.hpp"
#include "pingus/globals.hpp"
#include "pingus/plf_res_mgr.hpp"
#include "pingus/resource.hpp"

namespace pingus {


template<class C>
void write(std::ostream& out, const C& value)
{
  out.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

template<class C>
void read(std::istream& out, C& value)
{
  out.read(reinterpret_cast<char*>(&value), sizeof(value));
}

void write_events(std::ostream& out, const std::vector<input::Event>& events)
{
  write(out, events.size());
  for(std::vector<input::Event>::const_iterator i = events.begin();
      i != events.end();
      ++i)
  {
    write(out, *i);
  }
}

void read_events(std::istream& out, std::vector<input::Event>& events)
{
  std::vector<input::Event>::size_type len;
  read(out, len);
  for(std::vector<input::Event>::size_type i = 0; i < len; ++i)
  {
    input::Event event;
    read(out, event);
    events.push_back(event);
  }
}

void read_event(std::istream& out, input::Event& event)
{
  read(out, event.type);
  switch(event.type)
  {
    case input::BUTTON_EVENT_TYPE:
      read(out, event.button.name);
      read(out, event.button.state);
      break;

    case input::POINTER_EVENT_TYPE:
      read(out, event.pointer.name);
      read(out, event.pointer.x);
      read(out, event.pointer.y);
      break;

    case input::AXIS_EVENT_TYPE:
      read(out, event.axis.name);
      read(out, event.axis.dir);
      break;

    case input::SCROLLER_EVENT_TYPE:
      read(out, event.scroll.name);
      read(out, event.scroll.x_delta);
      read(out, event.scroll.y_delta);
      break;

    case input::KEYBOARD_EVENT_TYPE:
      read(out, event.keyboard);
      break;

    default:
      assert(!"Unknown Event type");
  }
}

void write_event(std::ostream& out, const input::Event& event)
{
  write(out, event.type);
  switch(event.type)
  {
    case input::BUTTON_EVENT_TYPE:
      write(out, event.button.name);
      write(out, event.button.state);
      break;

    case input::POINTER_EVENT_TYPE:
      write(out, event.pointer.name);
      write(out, event.pointer.x);
      write(out, event.pointer.y);
      break;

    case input::AXIS_EVENT_TYPE:
      write(out, event.axis.name);
      write(out, event.axis.dir);
      break;

    case input::SCROLLER_EVENT_TYPE:
      write(out, event.scroll.name);
      write(out, event.scroll.x_delta);
      write(out, event.scroll.y_delta);
      break;

    case input::KEYBOARD_EVENT_TYPE:
      write(out, event.keyboard);
      break;

    default:
      assert(!"Unknown Event type");
  }
}

ScreenManager* ScreenManager::instance_ = nullptr;

ScreenManager::ScreenManager(input::Manager& arg_input_manager,
                             input::ControllerPtr arg_input_controller) :
  input_manager(arg_input_manager),
  input_controller(arg_input_controller),
  display_gc(new DrawingContext()),
  fps_counter(),
  cursor(),
  screens(),
  mouse_pos(),
  record_input(false),
  playback_input(false)
{
  assert(instance_ == nullptr);
  instance_ = this;

  cursor = Sprite("core/cursors/animcross");
  fps_counter = std::make_unique<FPSCounter>();
}

ScreenManager::~ScreenManager()
{
  instance_ = nullptr;
}

void
ScreenManager::display()
{
  log_info("ScreenManager: entering display loop");
  show_software_cursor(globals::software_cursor);

  Uint32 last_ticks = SDL_GetTicks();
  float previous_frame_time;
  std::vector<input::Event> events;

  while (!screens.empty())
  {
    events.clear();

    // Get time and update input::Events
    if (playback_input)
    {
      // Get Time
      read(std::cin, previous_frame_time);

      // Update InputManager so that SDL_QUIT and stuff can be
      // handled, even if the basic events are taken from record
      input_manager.update(previous_frame_time);
      input_controller->clear_events();
      read_events(std::cin, events);
    }
    else
    {
      // Get Time
      Uint32 ticks = SDL_GetTicks();
      previous_frame_time  = float(ticks - last_ticks)/1000.0f;
      last_ticks = ticks;

      // Update InputManager and get Events
      input_manager.update(previous_frame_time);
      input_controller->poll_events(events);
    }

    if (record_input)
    {
      write(std::cerr, previous_frame_time);
      write_events(std::cerr, events);
    }

    if (globals::software_cursor)
      cursor.update(previous_frame_time);

    // previous frame took more than one second
    if (previous_frame_time > 1.0)
    {
      if (globals::developer_mode)
        log_warn("ScreenManager: previous frame took longer than 1 second ({} sec.), ignoring and doing frameskip", previous_frame_time);
    }
    else
    {
      update(previous_frame_time, events);

      // cap the framerate at the desired value
      // figure out how long this frame took
      float current_frame_time = float(SDL_GetTicks() - last_ticks) / 1000.0f;
      // idly delay if this frame didn't last long enough to
      // achieve <desired_fps> frames per second
      if (current_frame_time < 1.0f / globals::desired_fps) {
        Uint32 sleep_time = static_cast<Uint32>(1000 *((1.0f / globals::desired_fps) - current_frame_time));
        SDL_Delay(sleep_time);
      }
    }
  }
}

void
ScreenManager::update(float delta, const std::vector<input::Event>& events)
{
  ScreenPtr last_screen = get_current_screen();

  // Will be triggered when pop_all_screens() is called by pressing window close button
  if (!last_screen)
    return;

  for(const auto& event : events)
  {
    if (event.type == input::POINTER_EVENT_TYPE && event.pointer.name == input::STANDARD_POINTER)
      mouse_pos = Vector2i(static_cast<int>(event.pointer.x),
                           static_cast<int>(event.pointer.y));

    last_screen->update(event);

    if (last_screen != get_current_screen())
    {
      fade_over(last_screen, get_current_screen());
      return;
    }
  }

  last_screen->update(delta);
  if (last_screen != get_current_screen())
  {
    fade_over(last_screen, get_current_screen());
    return;
  }

  // Draw screen to DrawingContext
  get_current_screen()->draw(*display_gc);

  // Render the DrawingContext to the screen
  display_gc->render(*Display::get_framebuffer(), Rect(Vector2i(0,0), Size(Display::get_width(),
                                                                           Display::get_height())));
  display_gc->clear();

  // Draw the mouse pointer
  if (globals::software_cursor)
    cursor.render(mouse_pos.x, mouse_pos.y, *Display::get_framebuffer());

  // Draw FPS Counter
  if (globals::print_fps)
  {
    fps_counter->draw();
    if (globals::developer_mode)
    {
      fonts::pingus_small.render(origin_center, Display::get_width()/2, 60,
                                         "Developer Mode", *Display::get_framebuffer());
    }
  }
  else if (globals::developer_mode)
  {
    fonts::pingus_small.render(origin_center, Display::get_width()/2, 35,
                                       "Developer Mode", *Display::get_framebuffer());
  }

  Display::flip_display();
}

ScreenPtr
ScreenManager::get_current_screen()
{
  if (screens.empty())
    return ScreenPtr();
  else
    return screens.back();
}

ScreenManager*
ScreenManager::instance()
{
  return instance_;
}

void
ScreenManager::push_screen(ScreenPtr screen)
{
  screens.push_back(screen);
  screen->on_startup();
}

void
ScreenManager::pop_screen()
{
  screens.pop_back();

  // Purge unused textures and sprite metadata now that the popped
  // screen's destructors have run and its sprites have been released.
  Sprite::purge_cache();
  Resource::clear_cache();
  PLFResMgr::clear();

  if (!screens.empty())
  {
    if (screens.back()->get_size() != Display::get_size())
      screens.back()->resize(Display::get_size());
    screens.back()->on_startup();
  }
}

void
ScreenManager::pop_all_screens()
{
  screens.clear();
}

void
ScreenManager::replace_screen(ScreenPtr screen)
{
  // Overwriting screens.back() destroys the old ScreenPtr, running
  // destructors for the outgoing screen and all its sprites.
  screens.back() = screen;

  // Purge unused textures and level data now that the old screen's sprites are gone.
  Sprite::purge_cache();
  Resource::clear_cache();
  PLFResMgr::clear();

  if (screens.back()->get_size() != Display::get_size())
  {
    screens.back()->resize(Display::get_size());
  }

  screens.back()->on_startup();
}

void
ScreenManager::fade_over(ScreenPtr old_screen, ScreenPtr new_screen)
{
  if (!old_screen.get() || !new_screen.get())
    return;

  Uint32 last_ticks = SDL_GetTicks();
  float progress = 0.0f;
  Framebuffer& fb = *Display::get_framebuffer();
  while (progress <= 1.0f)
  {
    int border_x = static_cast<int>(static_cast<float>(Display::get_width()/2)  * (1.0f - progress));
    int border_y = static_cast<int>(static_cast<float>(Display::get_height()/2) * (1.0f - progress));

    old_screen->draw(*display_gc);
    display_gc->render(fb, Rect(Vector2i(0,0), Size(Display::get_width(),
                                                    Display::get_height())));
    display_gc->clear();

    fb.push_cliprect(Rect(Vector2i(0 + border_x, 0 + border_y),
                          Size(Display::get_width()  - 2*border_x,
                               Display::get_height() - 2*border_y)));

    new_screen->draw(*display_gc);
    display_gc->render(*Display::get_framebuffer(), Rect(Vector2i(0,0), Size(Display::get_width(),
                                                                            Display::get_height())));
    display_gc->clear();

    fb.pop_cliprect();
    fb.flip();
    display_gc->clear();

    progress = static_cast<float>(SDL_GetTicks() - last_ticks)/1000.0f * 2.0f;
  }

  input_manager.refresh();
}

void
ScreenManager::resize(const Size& size)
{
  display_gc->set_rect(Rect(Vector2i(0, 0), size));

  // The other screens will get resized when they become the current screen
  get_current_screen()->resize(size);
}

void
ScreenManager::show_software_cursor(bool visible)
{
  globals::software_cursor = visible;

  if (globals::software_cursor)
  {
    SDL_ShowCursor(SDL_DISABLE);
  }
  else
  {
    SDL_ShowCursor(SDL_ENABLE);
  }
}


} // namespace pingus

// EOF
