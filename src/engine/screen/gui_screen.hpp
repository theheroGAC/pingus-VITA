// src/engine/screen/gui_screen.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2000 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_SCREEN_GUI_SCREEN_HPP
#define HEADER_PINGUS_ENGINE_SCREEN_GUI_SCREEN_HPP

#include <memory>

#include "engine/screen/screen.hpp"
#include "fwd.hpp"

namespace pingus {

class GUIScreen : public Screen
{
protected:
  std::unique_ptr<gui::GUIManager> gui_manager;

public:
  GUIScreen ();
  virtual ~GUIScreen ();

  /** Draw this screen */
  virtual void draw_foreground (DrawingContext&) {}
  virtual void draw_background (DrawingContext&) {}
  virtual void draw(DrawingContext& gc) override;

  virtual void update (const input::Event& event) override;
  virtual void update (float) override;

  virtual void on_startup() override;

  virtual void on_pause_press () {}
  virtual void on_single_step_press () {}
  virtual void on_fast_forward_press () {}
  virtual void on_armageddon_press () {}
  virtual void on_escape_press () {}
  virtual void on_action_up_press() {}
  virtual void on_action_down_press() {}

  virtual void on_pause_release () {}
  virtual void on_single_step_release () {}
  virtual void on_fast_forward_release () {}
  virtual void on_armageddon_release () {}
  virtual void on_escape_release () {}
  virtual void on_action_up_release() {}
  virtual void on_action_down_release() {}

  virtual void on_action_axis_move (float) {}

  virtual void resize(const Size& size) override;

private:
  void process_button_event (const input::ButtonEvent& event);

  GUIScreen (const GUIScreen&);
  GUIScreen& operator= (const GUIScreen&);
};


} // namespace pingus
#endif

// EOF
