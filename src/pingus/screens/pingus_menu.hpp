// src/pingus/screens/pingus_menu.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1999 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_PINGUS_SCREENS_PINGUS_MENU_HPP
#define HEADER_PINGUS_PINGUS_SCREENS_PINGUS_MENU_HPP

#include <memory>

#include "engine/display/sprite.hpp"
#include "engine/screen/gui_screen.hpp"
#include "pingus/fonts.hpp"

namespace pingus {

namespace gui {
class GUIManager;
} // namespace gui

class SurfaceButton;
class GameDelta;
class LayerManager;
class MenuButton;

class PingusMenu : public GUIScreen
{
public:
  bool is_init;
  std::string hint;
  std::string help;
  float text_scroll_offset;

private:
  std::unique_ptr<LayerManager> background;
  Sprite logo;

  MenuButton* start_button;
  MenuButton* quit_button;
#ifndef DISABLE_EDITOR
  MenuButton* editor_button;
#endif
  MenuButton* contrib_button;
  MenuButton* options_button;
  MenuButton* credits_button;

  void show_credits();

  void do_quit();
  void do_start(const std::string &filename);
#ifndef DISABLE_EDITOR
  void do_edit();
#endif

  void create_background(const Size& size);

  void layout_buttons(const Size& size);

public:
  PingusMenu();
  ~PingusMenu();

  void on_click(MenuButton* button);
  void set_hint(const std::string& str);

  // Load all images and other stuff for the menu
  void on_escape_press();
  void draw_background(DrawingContext& gc);

  void update(float delta);

  void resize(const Size& size);

private:
  PingusMenu(const PingusMenu&);
  PingusMenu &operator=(const PingusMenu&);
};

} // namespace pingus
#endif

// EOF
