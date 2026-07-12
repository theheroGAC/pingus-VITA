// src/pingus/screens/option_menu.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2007 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_PINGUS_SCREENS_OPTION_MENU_HPP
#define HEADER_PINGUS_PINGUS_SCREENS_OPTION_MENU_HPP

#include <memory>
#include <map>
#include <vector>

#include "engine/display/sprite.hpp"
#include "engine/gui/rect_component.hpp"
#include "engine/gui/surface_button.hpp"
#include "engine/screen/gui_screen.hpp"
#include "pingus/components/label.hpp"

namespace pingus {


class CheckBox;
class SliderBox;

class OptionMenu : public GUIScreen
{
private:
  Sprite m_background;
  Sprite m_blackboard;
  gui::SurfaceButton* ok_button;
  int x_pos;
  int y_pos;

  struct Option {
    Label*         label;
    gui::RectComponent* control;

    Option(Label* label_, gui::RectComponent* control_) :
      label(label_), control(control_)
    {}
  };

  typedef std::vector<Option> Options;
  Options options;

  CheckBox* fullscreen_box;
  CheckBox* software_cursor_box;
  CheckBox* autoscroll_box;
  CheckBox* dragdrop_scroll_box;
  CheckBox* mousegrab_box;
  CheckBox* printfps_box;

  SliderBox* master_volume_box;
  SliderBox* sound_volume_box;
  SliderBox* music_volume_box;

  //Label* defaults_label;
  //CheckBox* defaults_box;

public:
  OptionMenu();
  ~OptionMenu();

  void draw_background (DrawingContext& gc);
  void on_escape_press ();

  void add_item(const std::string& label, std::unique_ptr<gui::RectComponent> control);

  void resize(const Size&);
  void close_screen();

  void on_software_cursor_change(bool v);
  void on_fullscreen_change(bool v);
  void on_autoscroll_change(bool v);
  void on_drag_drop_scrolling_change(bool v);
  void on_fastmode_change(bool v);
  void on_mousegrab_change(bool v);
  void on_printfps_change(bool v);

  void on_master_volume_change(int v);
  void on_sound_volume_change(int v);
  void on_music_volume_change(int v);

  void on_resolution_change(const std::string& str);
  void on_renderer_change(const std::string& str);
  void on_language_change(const std::string& str);

  void save_language();

private:
  OptionMenu (const OptionMenu&);
  OptionMenu& operator= (const OptionMenu&);
};


} // namespace pingus
#endif

// EOF
