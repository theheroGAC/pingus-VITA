// src/pingus/worldmap/level_dot.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2002 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/worldmap/level_dot.hpp"

#include "engine/display/drawing_context.hpp"
#include "engine/input/control.hpp"
#include "engine/screen/screen_manager.hpp"
#include "pingus/fonts.hpp"
#include "pingus/globals.hpp"
#include "pingus/plf_res_mgr.hpp"
#include "pingus/savegame_manager.hpp"
#include "pingus/screens/start_screen.hpp"

#include "pingus/worldmap/worldmap.hpp"

namespace pingus::worldmap {

LevelDot::LevelDot(const FileReader& reader) :
  Dot(reader.read_section("dot")),
  green_dot_sur("core/worldmap/dot_green"),
  red_dot_sur("core/worldmap/dot_red"),
  inaccessible_dot_sur("core/worldmap/dot_invalid"),
  highlight_green_dot_sur("core/worldmap/dot_green_hl"),
  highlight_red_dot_sur("core/worldmap/dot_red_hl"),
  plf()
{
  std::string resname;
  reader.read_string("levelname", resname);

  plf = PLFResMgr::load_plf(resname);
}

void
LevelDot::draw(DrawingContext& gc)
{
  bool highlight = m_highlight;

  Savegame* savegame = SavegameManager::instance()->get(plf.get_resname());
  if (savegame
      && (savegame->get_status() == Savegame::FINISHED
          || savegame->get_status() == Savegame::ACCESSIBLE))
  {
    if (savegame->get_status() == Savegame::FINISHED)
      if (highlight)
      {
        gc.draw (highlight_green_dot_sur, pos);
      }
      else
      {
        gc.draw (green_dot_sur, pos);
      }
    else
      if (highlight)
        gc.draw (highlight_red_dot_sur, pos);
      else
        gc.draw (red_dot_sur, pos);
  }
  else
  {
    gc.draw (inaccessible_dot_sur, pos);
  }
}

void
LevelDot::update(float /*delta*/)
{
}

void
LevelDot::on_click()
{
  //log_info("Starting level: {}", levelname);
  ScreenManager::instance()->push_screen(std::make_shared<StartScreen>(plf));
}

bool
LevelDot::is_finished() const
{
  Savegame* savegame = SavegameManager::instance()->get(plf.get_resname());
  if (savegame && savegame->get_status() == Savegame::FINISHED)
    return true;
  else
    return false;
}

bool
LevelDot::is_accessible() const
{
  Savegame* savegame = SavegameManager::instance()->get(plf.get_resname());
  if (savegame && savegame->get_status() != Savegame::NONE)
    return true;
  else
    return false;
}

void
LevelDot::draw_hover(DrawingContext& gc)
{
  if (is_accessible())
  {
    gc.print_center(pingus::fonts::pingus_small,
                    Vector2i(static_cast<int>(pos.x),
                             static_cast<int>(pos.y) - 44),
                    get_plf().get_levelname(),
                    10000);
  }
  else
  {
    gc.print_center(pingus::fonts::pingus_small,
                    Vector2i(static_cast<int>(pos.x),
                             static_cast<int>(pos.y) - 44),
                    "???",
                    10000);
  }

  if (globals::developer_mode)
  {
    gc.print_center(pingus::fonts::pingus_small,
                    Vector2i(static_cast<int>(pos.x), static_cast<int>(pos.y) - 70),
                    get_plf().get_resname(),
                    10000);
  }
}

void
LevelDot::unlock()
{
  Savegame* savegame = SavegameManager::instance()->get(plf.get_resname());
  if (savegame == nullptr || savegame->get_status() == Savegame::NONE)
  {
    Savegame savegame_(plf.get_resname(),
                       Savegame::ACCESSIBLE,
                       0,
                       0);
    SavegameManager::instance()->store(savegame_);
  }
  else
  {
  }
}

} // namespace pingus::worldmap

// EOF
