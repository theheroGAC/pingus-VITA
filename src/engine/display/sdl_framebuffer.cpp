// src/engine/display/sdl_framebuffer.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "engine/display/sdl_framebuffer.hpp"

#include "engine/display/display.hpp"
#include "engine/display/sdl_framebuffer_surface_impl.hpp"
#include "util/log.hpp"

namespace pingus {

SDLFramebuffer::SDLFramebuffer() :
  m_window(nullptr),
  m_renderer(nullptr),
  m_fullscreen(false),
  m_resizable(false),
  cliprect_stack()
{
}

SDLFramebuffer::~SDLFramebuffer()
{
  if (m_renderer)
  {
    SDL_DestroyRenderer(m_renderer);
    m_renderer = nullptr;
  }
  if (m_window)
  {
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }
}

FramebufferSurface
SDLFramebuffer::create_surface(const Surface& surface)
{
  return FramebufferSurface(new SDLFramebufferSurfaceImpl(m_renderer, surface.get_surface()));
}

void
SDLFramebuffer::draw_surface(const FramebufferSurface& surface, Vector2i pos)
{
  SDLFramebufferSurfaceImpl* impl = dynamic_cast<SDLFramebufferSurfaceImpl*>(surface.get_impl());

  // dst uses the original (game-visible) dimensions so positioning is correct.
  // When the texture was downscaled (scale != 1), passing src=nullptr tells SDL
  // to render the entire (smaller) texture stretched to fill the dst rect,
  // reconstructing the image at the expected size.
  SDL_Rect dst = { pos.x, pos.y, impl->get_width(), impl->get_height() };
  SDL_RenderCopy(m_renderer, impl->get_texture(), nullptr, &dst);
}

void
SDLFramebuffer::draw_surface(const FramebufferSurface& surface, const Rect& srcrect, Vector2i pos)
{
  SDLFramebufferSurfaceImpl* impl = dynamic_cast<SDLFramebufferSurfaceImpl*>(surface.get_impl());

  const float sx = impl->get_scale_x();
  const float sy = impl->get_scale_y();

  // Translate the source rect from original coordinates to actual texture
  // coordinates.  For most textures sx==sy==1 so this is a no-op.
  // For downscaled textures (large worldmap backgrounds on Wii) this maps the
  // requested region to the corresponding pixel in the smaller texture.
  SDL_Rect src = {
    static_cast<int>(srcrect.left  * sx),
    static_cast<int>(srcrect.top   * sy),
    static_cast<int>(srcrect.get_width()  * sx),
    static_cast<int>(srcrect.get_height() * sy)
  };
  // dst keeps the original (logical) dimensions so the image renders at the
  // correct size and position on screen.
  SDL_Rect dst = {
    pos.x, pos.y,
    srcrect.get_width(), srcrect.get_height()
  };
  SDL_RenderCopy(m_renderer, impl->get_texture(), &src, &dst);
}

void
SDLFramebuffer::draw_line(Vector2i pos1, Vector2i pos2, Color color)
{
  SDL_SetRenderDrawBlendMode(m_renderer,
    color.a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
  SDL_RenderDrawLine(m_renderer, pos1.x, pos1.y, pos2.x, pos2.y);
}

void
SDLFramebuffer::draw_rect(const Rect& rect_, Color color)
{
  Rect r = rect_;
  r.normalize();

  SDL_SetRenderDrawBlendMode(m_renderer,
    color.a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

  // Four independent SDL_RenderDrawLine calls instead of SDL_RenderDrawLines
  // with a 5-point closed path.
  //
  // With SDL_RenderDrawLines, when a point is off-screen SDL clips each
  // segment's endpoint independently -- but the "current position" between
  // consecutive segments does not reset.  The closing segment from pt[3] back
  // to pt[0] therefore connects two DIFFERENT clipped edge positions, producing
  // a stray diagonal line across the screen when the rectangle is partially
  // outside the viewport.  Independent line calls each clip to the viewport
  // edge cleanly with no connection artefacts between them.
  SDL_RenderDrawLine(m_renderer, r.left,    r.top,      r.right-1, r.top);
  SDL_RenderDrawLine(m_renderer, r.left,    r.bottom-1, r.right-1, r.bottom-1);
  SDL_RenderDrawLine(m_renderer, r.left,    r.top,      r.left,    r.bottom-1);
  SDL_RenderDrawLine(m_renderer, r.right-1, r.top,      r.right-1, r.bottom-1);
}

void
SDLFramebuffer::fill_rect(const Rect& rect_, Color color)
{
  Rect r = rect_;
  r.normalize();
  SDL_Rect sdl_rect = { r.left, r.top, r.get_width(), r.get_height() };
  SDL_SetRenderDrawBlendMode(m_renderer,
    color.a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
  SDL_RenderFillRect(m_renderer, &sdl_rect);
}

void
SDLFramebuffer::flip()
{
  SDL_RenderPresent(m_renderer);

  // Clear the back buffer for the next frame.  The old surface-based pipeline
  // implicitly cleared every frame because every pixel was overwritten by
  // fresh blits.  With SDL_Renderer the back buffer is NOT automatically wiped
  // between frames, so moving elements (e.g. the minimap viewport indicator)
  // leave accumulating ghost trails without this call.
  SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
  SDL_RenderClear(m_renderer);
}

void
SDLFramebuffer::update_rects(const std::vector<Rect>& /*rects*/)
{
  // Not needed with the SDL_Renderer path -- flip() handles everything.
}

void
SDLFramebuffer::set_video_mode(const Size& size, bool fullscreen, bool resizable)
{
  m_fullscreen = fullscreen;
  m_resizable  = resizable;

  if (m_window == nullptr)
  {
    Uint32 flags = 0;
    if (fullscreen)
      flags |= SDL_WINDOW_FULLSCREEN;
    else if (resizable)
      flags |= SDL_WINDOW_RESIZABLE;

    m_window = SDL_CreateWindow("Pingus " VERSION,
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                size.width, size.height,
                                flags);
    if (m_window == nullptr)
    {
      log_error("Unable to create window: {}", SDL_GetError());
      exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    m_renderer = SDL_CreateRenderer(m_window, -1,
                                    SDL_RENDERER_ACCELERATED |
                                    SDL_RENDERER_PRESENTVSYNC);
    if (m_renderer == nullptr)
      m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);

    if (m_renderer == nullptr)
    {
      log_error("Unable to create renderer: {}", SDL_GetError());
      exit(1);
    }

    SDL_RenderSetLogicalSize(m_renderer,
                             Display::LOGICAL_WIDTH,
                             Display::LOGICAL_HEIGHT);

    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
  }
  else
  {
    if (fullscreen)
      SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
    else
    {
      SDL_SetWindowFullscreen(m_window, 0);
      SDL_SetWindowSize(m_window, size.width, size.height);
    }
  }
}

Size
SDLFramebuffer::get_size() const
{
  return Size(Display::LOGICAL_WIDTH, Display::LOGICAL_HEIGHT);
}

bool
SDLFramebuffer::is_fullscreen() const
{
  if (!m_window) return m_fullscreen;
  return (SDL_GetWindowFlags(m_window) &
          (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
}

bool
SDLFramebuffer::is_resizable() const
{
  if (!m_window) return m_resizable;
  return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_RESIZABLE) != 0;
}

void
SDLFramebuffer::push_cliprect(const Rect& rect)
{
  Rect logical = rect;

  if (!cliprect_stack.empty())
  {
    const Rect& cur = cliprect_stack.back();
    logical = Rect(
      std::max(logical.left,   cur.left),
      std::max(logical.top,    cur.top),
      std::min(logical.right,  cur.right),
      std::min(logical.bottom, cur.bottom)
    );
  }

  cliprect_stack.push_back(logical);

  SDL_Rect sdl_rect;
  if (logical.left >= logical.right || logical.top >= logical.bottom)
    sdl_rect = { 0, 0, 0, 0 };
  else
    sdl_rect = { logical.left, logical.top, logical.get_width(), logical.get_height() };

  SDL_RenderSetClipRect(m_renderer, &sdl_rect);
}

void
SDLFramebuffer::pop_cliprect()
{
  cliprect_stack.pop_back();

  if (cliprect_stack.empty())
  {
    SDL_RenderSetClipRect(m_renderer, nullptr);
  }
  else
  {
    const Rect& cr = cliprect_stack.back();
    SDL_Rect sdl_rect;
    if (cr.left >= cr.right || cr.top >= cr.bottom)
      sdl_rect = { 0, 0, 0, 0 };
    else
      sdl_rect = { cr.left, cr.top, cr.get_width(), cr.get_height() };
    SDL_RenderSetClipRect(m_renderer, &sdl_rect);
  }
}


} // namespace pingus

// EOF
