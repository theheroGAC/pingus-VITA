// src/editor/inputbox.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2007 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "editor/inputbox.hpp"

#include "util/log.hpp"
#include "util/utf8.hpp"
#include "engine/display/drawing_context.hpp"
#include "pingus/fonts.hpp"

namespace pingus::editor {

Inputbox::Inputbox() :
  m_text(),
  m_faulty_input(false),
  on_change(),
  on_enter()
{
}

Inputbox::Inputbox(const Rect& rect_) :
  RectComponent(rect_),
  m_text(),
  m_faulty_input(false),
  on_change(),
  on_enter()
{
}

void
Inputbox::draw(DrawingContext& gc)
{
  Color const bg_color = m_faulty_input ? Color(255, 128, 128) : Color(255, 255, 255);

  gc.draw_fillrect(rect, bg_color);
  gc.draw_rect(rect, has_focus() ? Color(255, 128, 0) : Color(0, 0, 0));

  gc.print_left(pingus::fonts::verdana11,
                Vector2i(rect.left + 5,
                         rect.top + rect.get_height()/2 - pingus::fonts::verdana11.get_height()/2),
                m_text);
}

void
Inputbox::set_text(const std::string& text_)
{
  m_text = text_;
}

void
Inputbox::on_key_pressed(const input::KeyboardEvent& ev)
{
  if (ev.keysym.sym == SDLK_BACKSPACE) // backspace
  {
    if (!m_text.empty())
    {
      m_text = m_text.substr(0, m_text.size() - 1);
      try {
        if (on_change) on_change(m_text);
        m_faulty_input = false;
      } catch (const std::exception& err) {
        m_faulty_input = true;
        log_debug("Inputbox: on_change failed: {}", err.what());
      }
    }
  }
  else if (ev.keysym.sym == SDLK_RETURN) // enter
  {
    try {
      if (on_change) on_change(m_text);
      if (on_enter) on_enter(m_text);
      m_faulty_input = false;
    } catch (const std::exception& err) {
      m_faulty_input = true;
      log_debug("Inputbox: on_change/on_enter failed: {}", err.what());
    }
  }
  else
  {
    if (ev.unicode)
    {
      m_text += UTF8::encode_utf8(ev.unicode);
      try {
        if (on_change) on_change(m_text);
        m_faulty_input = false;
      } catch (const std::exception& err) {
        m_faulty_input = true;
        log_debug("Inputbox: on_change failed: {}", err.what());
      }
    }
  }
}

} // namespace pingus::editor

// EOF
