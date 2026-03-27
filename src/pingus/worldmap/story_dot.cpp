// src/pingus/worldmap/story_dot.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1998-2011 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pingus/worldmap/story_dot.hpp"

#include "engine/display/drawing_context.hpp"
#include "engine/screen/screen_manager.hpp"
#include "pingus/fonts.hpp"
#include "pingus/screens/story_screen.hpp"
#include "pingus/stat_manager.hpp"
#include "util/file_reader.hpp"
#include "util/file_reader.hpp"
#include "util/log.hpp"
#include "util/pathname.hpp"

namespace pingus::worldmap {

StoryDot::StoryDot(const FileReader& reader) :
  Dot(reader.read_section("dot")),
  m_story_dot_highlight("core/worldmap/story_dot_highlight"),
  m_story_dot("core/worldmap/story_dot"),
  m_inaccessible_dot("core/worldmap/dot_invalid"),
  m_name(),
  m_story(),
  m_credits(false),
  m_accessible(false),
  m_auto_play(false)
{
  reader.read_string("name", m_name);
  reader.read_string("story", m_story);
  reader.read_bool("credits", m_credits);
  reader.read_bool("accessible", m_accessible);
  reader.read_bool("auto-play", m_auto_play);
}

void
StoryDot::draw(DrawingContext& gc)
{
  if (m_accessible)
  {
    if (m_highlight)
      gc.draw(m_story_dot_highlight, pos);
    else
      gc.draw(m_story_dot, pos);
  }
  else
  {
    gc.draw(m_inaccessible_dot, pos);
  }
}

void
StoryDot::draw_hover(DrawingContext& gc)
{
  if (m_accessible)
  {
    gc.print_center(pingus::fonts::pingus_small,
                    Vector2i(static_cast<int>(pos.x),
                             static_cast<int>(pos.y) - 44),
                    m_name,
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
}

void
StoryDot::update(float /*delta*/)
{
}

void
StoryDot::on_click()
{
  try
  {
    FileReader reader = FileReader::parse(Pathname(m_story, Pathname::DATA_PATH));
    ScreenManager::instance()->push_screen(std::make_shared<StoryScreen>(reader, m_credits));
  }
  catch(const std::exception& err)
  {
    log_error("{}", err.what());
  }
}

void
StoryDot::check_auto_play()
{
  if (!m_accessible || !m_auto_play)
    return;

  // Use the story path as a unique key so each story only auto-plays once.
  std::string stat_key = "story-auto-played:" + m_story;
  bool already_played = false;
  StatManager::instance()->get_bool(stat_key, already_played);
  if (already_played)
    return;

  StatManager::instance()->set_bool(stat_key, true);

  try
  {
    FileReader reader = FileReader::parse(Pathname(m_story, Pathname::DATA_PATH));
    ScreenManager::instance()->push_screen(std::make_shared<StoryScreen>(reader, m_credits));
  }
  catch(const std::exception& err)
  {
    log_error("{}", err.what());
  }
}

} // namespace pingus::worldmap

// EOF
