// src/engine/display/display.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2000 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "engine/display/display.hpp"

#include <stdexcept>
#include <algorithm>

#include "engine/display/sdl_framebuffer.hpp"
#include "engine/screen/screen_manager.hpp"
#include "pingus/globals.hpp"
#ifdef HAVE_OPENGL
#  include "engine/display/opengl/opengl_framebuffer.hpp"
#endif
#include "engine/display/null_framebuffer.hpp"
#include "util/log.hpp"

namespace pingus {


std::unique_ptr<Framebuffer> Display::s_framebuffer;

void
Display::flip_display()
{
  return s_framebuffer->flip();
}

int
Display::get_width()
{
  return LOGICAL_WIDTH;
}

int
Display::get_height()
{
  return LOGICAL_HEIGHT;
}

Size
Display::get_size()
{
  return Size(LOGICAL_WIDTH, LOGICAL_HEIGHT);
}

Size
Display::get_physical_size()
{
  return s_framebuffer.get() ? s_framebuffer->get_size() : Size(LOGICAL_WIDTH, LOGICAL_HEIGHT);
}

void
Display::resize(const Size& size_)
{
  Size size(size_);

  // Limit Window size to some reasonable minimum
  if (size.width  < 640) size.width  = 640;
  if (size.height < 480) size.height = 480;

  // FIXME: Calling this causes horrible flicker, since the screen
  // goes black on a size change. Seems to be an SDL issue.
  // This call  also shouldn't be part of ScreenManager, but Framebuffer/Display internal
  Display::set_video_mode(size, is_fullscreen(), true);

  if (ScreenManager::instance())
    ScreenManager::instance()->resize(Size(LOGICAL_WIDTH, LOGICAL_HEIGHT));
}

bool
Display::is_fullscreen()
{
  return s_framebuffer->is_fullscreen();
}

bool
Display::is_resizable()
{
  return s_framebuffer->is_resizable();
}

void
Display::create_window(FramebufferType framebuffer_type, const Size& size, bool fullscreen, bool resizable)
{
  assert(!s_framebuffer.get());

  log_info("{} {}x{} {}", framebuffer_type_to_string(framebuffer_type), size.width, size.height, (fullscreen?"fullscreen":"window"));

  switch (framebuffer_type)
  {
    case OPENGL_FRAMEBUFFER:
#ifdef HAVE_OPENGL
      s_framebuffer = std::unique_ptr<Framebuffer>(new OpenGLFramebuffer());
      s_framebuffer->set_video_mode(size, fullscreen, resizable);
#else
      throw std::runtime_error("OpenGL support was not compiled in");
#endif
      break;

    case NULL_FRAMEBUFFER:
      s_framebuffer = std::unique_ptr<Framebuffer>(new NullFramebuffer());
      s_framebuffer->set_video_mode(size, fullscreen, resizable);
      break;

    case SDL_FRAMEBUFFER:
      s_framebuffer = std::unique_ptr<Framebuffer>(new SDLFramebuffer());
      s_framebuffer->set_video_mode(size, fullscreen, resizable);
      break;

    default:
      assert(!"Unknown framebuffer_type");
      break;
  }
}

void
Display::set_video_mode(const Size& size, bool fullscreen, bool resizable)
{
  if (fullscreen)
  {
    Size new_size = find_closest_fullscreen_video_mode(size);
    s_framebuffer->set_video_mode(new_size, fullscreen, resizable);
  }
  else
  {
    s_framebuffer->set_video_mode(size, fullscreen, resizable);
  }

  if (ScreenManager::instance())
  {
    // Always notify screens using logical resolution, not physical framebuffer
    // size. Screens lay out their content in logical coordinate space.
    ScreenManager::instance()->resize(Size(LOGICAL_WIDTH, LOGICAL_HEIGHT));
  }
}

Framebuffer*
Display::get_framebuffer()
{
  return s_framebuffer.get();
}

Size
Display::find_closest_fullscreen_video_mode(const Size& size)
{
  int num_modes = SDL_GetNumDisplayModes(0);
  if (num_modes <= 0)
    return size;

  int best_distance = -1;
  Size best_fit = size;

  for (int i = 0; i < num_modes; ++i)
  {
    SDL_DisplayMode mode;
    if (SDL_GetDisplayMode(0, i, &mode) != 0)
      continue;

    int this_distance = std::abs(size.width - mode.w) + std::abs(size.height - mode.h);
    if (best_distance == -1 || best_distance > this_distance)
    {
      best_distance  = this_distance;
      best_fit.width  = mode.w;
      best_fit.height = mode.h;
    }
  }

  return best_fit;
}

struct SortBySize
{
  bool operator()(const Size& lhs, const Size& rhs)
  {
    return lhs.get_area() < rhs.get_area();
  }
};

std::vector<Size>
Display::get_fullscreen_video_modes()
{
  std::vector<Size> video_modes;

  int num_modes = SDL_GetNumDisplayModes(0);
  if (num_modes <= 0)
  {
    // Fall back to hardcoded defaults
    log_warn("SDL_GetNumDisplayModes failed, falling back to hardcoded list");
    video_modes.push_back(Size( 640, 480)); // 4:3, VGA
    video_modes.push_back(Size( 800, 600)); // 4:3, PAL
    video_modes.push_back(Size(1024, 768)); // Nokia N770, N800
    video_modes.push_back(Size(1152, 864)); // 4:3, SVGA
    video_modes.push_back(Size(1280, 720)); // 16:9
    video_modes.push_back(Size(1280, 800)); // 16:10
    video_modes.push_back(Size(1280, 960)); // 4:3, XGA
    video_modes.push_back(Size(1280, 1024)); // 5:4
    video_modes.push_back(Size(1366,  768)); // ~16:9, Wide XGA
    video_modes.push_back(Size(1440, 900)); // 16:10
    video_modes.push_back(Size(1680, 1050)); // 16:10
    video_modes.push_back(Size(1600, 1200)); // 4:3, UXGA
    video_modes.push_back(Size(1920, 1080)); // 16:9, HD-TV, 1080p
    video_modes.push_back(Size(1920, 1200)); // 16:10
  }
  else
  {
    for (int i = 0; i < num_modes; ++i)
    {
      SDL_DisplayMode mode;
      if (SDL_GetDisplayMode(0, i, &mode) == 0)
        video_modes.push_back(Size(mode.w, mode.h));
    }
  }

  std::sort(video_modes.begin(), video_modes.end(), SortBySize());

  return video_modes;
}


} // namespace pingus

// EOF
