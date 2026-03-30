// src/pingus/screens/pingus_menu.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1999 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/screens/pingus_menu.hpp"

#ifndef DISABLE_EDITOR
#include "editor/editor_screen.hpp"
#endif

#include "engine/screen/screen_manager.hpp"
#include "engine/sound/sound.hpp"
#include "pingus/components/menu_button.hpp"
#include "pingus/layer_manager.hpp"
#include "pingus/plf_res_mgr.hpp"
#include "pingus/resource.hpp"
#include "pingus/screens/credits.hpp"
#include "pingus/screens/level_menu.hpp"
#include "pingus/screens/option_menu.hpp"
#include "pingus/screens/start_screen.hpp"
#include "pingus/worldmap/worldmap_screen.hpp"

namespace pingus {

PingusMenu::PingusMenu() :
  is_init(),
  hint(),
  help(),
  text_scroll_offset(),
  background(),
  logo(),
  start_button(),
  quit_button(),
#ifndef DISABLE_EDITOR
  editor_button(),
#endif
  contrib_button(),
  options_button(),
  credits_button()
{
  is_init = false;

  // Initialize buttons with dummy positions (0,0).
  // The actual layout is applied immediately after via layout_buttons()

  // Add buttons to GUI manager
  quit_button = gui_manager->create<MenuButton>(this, Vector2i(0, 0), "Exit", "..:: Bye, bye ::..");
  options_button = gui_manager->create<MenuButton>(this, Vector2i(0, 0), "Options", "..:: Configure the game ::..");
  contrib_button = gui_manager->create<MenuButton>(this, Vector2i(0, 0), "Levelsets", "..:: Play User Built levels ::..");
  start_button = gui_manager->create<MenuButton>(this, Vector2i(0, 0), "Story", "..:: Start the game ::..");
  credits_button = gui_manager->create<MenuButton>(this, Vector2i(0, 0), "Credits", "..:: Meet the penguins behind the penguins ::..");
#ifndef DISABLE_EDITOR
  editor_button = gui_manager->create<MenuButton>(this, Vector2i(0, 0), "Editor", "..:: Create your own levels ::..");
#endif

  logo = Sprite("core/misc/logo");

  // Apply initial layout
  create_background(Size(Display::get_width(), Display::get_height()));
  layout_buttons(Size(Display::get_width(), Display::get_height()));

  help = "..:: Ctrl-g: mouse grab   ::   F10: fps counter   ::   F11: "
         "fullscreen   ::   F12: screenshot ::..";

  sound::PingusSound::play_music("pingus-1.it");
}

PingusMenu::~PingusMenu()
{
}

void
PingusMenu::layout_buttons(const Size& size)
{
  const int center_x = size.width / 2;
  const int center_y = size.height / 2;
  const int x_offset = 125;
  const int row_spacing = 70;

  // Three rows, centered vertically as a group
  const Vector2i slot_tl(center_x - x_offset, center_y - 30);
  const Vector2i slot_tr(center_x + x_offset, center_y - 30);
  const Vector2i slot_ml(center_x - x_offset, center_y - 30 + row_spacing);
  const Vector2i slot_mr(center_x + x_offset, center_y - 30 + row_spacing);
  const Vector2i slot_bl(center_x - x_offset, center_y - 30 + row_spacing * 2);
  const Vector2i slot_br(center_x + x_offset, center_y - 30 + row_spacing * 2);
  const Vector2i slot_bc(center_x,            center_y - 30 + row_spacing * 2);

  // Row 1: Story | Options  (always)
  start_button->set_pos(slot_tl.x, slot_tl.y);
  options_button->set_pos(slot_tr.x, slot_tr.y);

  // Row 2: Levelsets | Credits  (always)
  contrib_button->set_pos(slot_ml.x, slot_ml.y);
  credits_button->set_pos(slot_mr.x, slot_mr.y);

#ifndef DISABLE_EDITOR
  // Row 3 with editor: Editor | Exit
  editor_button->set_pos(slot_bl.x, slot_bl.y);
  quit_button->set_pos(slot_br.x, slot_br.y);
#else
  // Row 3 without editor: Exit centered
  quit_button->set_pos(slot_bc.x, slot_bc.y);
#endif
}

void
PingusMenu::show_credits()
{
  ScreenManager::instance()->push_screen(std::make_shared<Credits>(
      Pathname("credits/pingus.credits", Pathname::DATA_PATH)));
}

void
PingusMenu::do_quit()
{
  ScreenManager::instance()->pop_screen();
}

void
PingusMenu::do_start(const std::string &filename)
{ // Start the story or worldmap mode
  sound::PingusSound::play_sound("letsgo");

  std::shared_ptr<worldmap::WorldmapScreen> worldmap_screen =
      std::make_shared<worldmap::WorldmapScreen>();
  worldmap_screen->load(Pathname(filename, Pathname::DATA_PATH));
  ScreenManager::instance()->push_screen(worldmap_screen);

}

#ifndef DISABLE_EDITOR
void PingusMenu::do_edit()
{ // Launch the level editor
  sound::PingusSound::stop_music();
  ScreenManager::instance()->push_screen(
      std::make_shared<editor::EditorScreen>());
}
#endif

void PingusMenu::on_escape_press()
{
  // FIXME: get_manager()->show_exit_menu ();
}

void
PingusMenu::draw_background(DrawingContext& gc)
{
  background->draw(gc);

  gc.draw(logo, Vector2i((gc.get_width() / 2) - (logo.get_width() / 2),
                         gc.get_height() / 2 - 280));

  gc.print_left(fonts::pingus_small,
                Vector2i(gc.get_width() / 2 - 400 + 25, gc.get_height() - 140),
                "Pingus " VERSION
                " - Copyright (C) 1998-2011 Ingo Ruhnke <grumbel@gmail.com>\n"
                "See the file AUTHORS for a complete list of contributors.\n"
                "Pingus comes with ABSOLUTELY NO WARRANTY. This is free "
                "software, and you are\n welcome to redistribute it under "
                "certain conditions; see the file LICENSE for details.\n");

  gc.draw_fillrect(Rect(0, Display::get_height() - 26, Display::get_width(),
                        Display::get_height()),
                   Color(0, 0, 0, 255));

  gc.print_center(
      fonts::pingus_small,
      Vector2i(gc.get_width() / 2,
               gc.get_height() - fonts::pingus_small.get_height() - 8),
      help);

  if (0) // display hint
  {
    gc.print_center(
        fonts::pingus_small,
        Vector2i(gc.get_width() / 2,
                 gc.get_height() - fonts::pingus_small.get_height()),
        hint);
  }
}

void
PingusMenu::on_click(MenuButton* button)
{
  if (button == start_button)
  {
    do_start("worldmaps/tutorial.worldmap");
  }
  else if (button == quit_button)
  {
    do_quit();
  }
#ifndef DISABLE_EDITOR
  else if (button == editor_button)
  {
    do_edit();
  }
#endif
  else if (button == contrib_button)
  {
    ScreenManager::instance()->push_screen(std::make_shared<LevelMenu>());
  }
  else if (button == credits_button)
  {
    show_credits();
  }
  else if (button == options_button)
  {
    ScreenManager::instance()->push_screen(std::make_shared<OptionMenu>());
  }
}

void
PingusMenu::set_hint(const std::string& str)
{
  hint = str;
}

void
PingusMenu::update(float delta)
{
  background->update(delta);
}

void
PingusMenu::create_background(const Size& size_)
{
  // Recreate the layer manager in the new size
  background = std::unique_ptr<LayerManager>(new LayerManager());

  Surface layer1 = Resource::load_surface("core/menu/layer1");
  Surface layer2 = Resource::load_surface("core/menu/layer2");
  Surface layer3 = Resource::load_surface("core/menu/layer3");
  Surface layer4 = Resource::load_surface("core/menu/layer4");
  Surface layer5 = Resource::load_surface("core/menu/layer5");

  int w = size_.width;
  int h = size_.height;

  Size base(800, 600);

  // We only need to scale the background main menu images if the screen
  // resolution is not default
  if (w != base.width ||
      h != base.height)
  {
    layer1 = layer1.scale(w, 185 * h / base.height);
    layer2 = layer2.scale(w, 362 * h / base.height);
    layer3 = layer3.scale(w, 306 * h / base.height);
    layer4 = layer4.scale(w, 171 * h / base.height);
    layer5 = layer5.scale(302 * w / base.width,
                          104 * h / base.height);

    background->add_layer(Sprite(layer1), 0, 0, 12, 0);
    background->add_layer(Sprite(layer2), 0, 150 * static_cast<float>(h) / static_cast<float>(base.height), 25, 0);
    background->add_layer(Sprite(layer3), 0, 200 * static_cast<float>(h) / static_cast<float>(base.height), 50, 0);
    background->add_layer(Sprite(layer4), 0, 429 * static_cast<float>(h) / static_cast<float>(base.height), 100, 0);
    background->add_layer(Sprite(layer5), 0, 500 * static_cast<float>(h) / static_cast<float>(base.height), 200, 0);
  }
  else
  {
    background->add_layer(Sprite(layer1), 0, 0, 12, 0);
    background->add_layer(Sprite(layer2), 0, 150, 25, 0);
    background->add_layer(Sprite(layer3), 0, 200, 50, 0);
    background->add_layer(Sprite(layer4), 0, 429, 100, 0);
    background->add_layer(Sprite(layer5), 0, 500, 200, 0);
  }
}

void
PingusMenu::resize(const Size& size_)
{
  GUIScreen::resize(size_);
  create_background(size_);
  layout_buttons(size_);
}

} // namespace pingus

// EOF
