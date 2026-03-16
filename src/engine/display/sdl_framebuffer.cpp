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

#include <algorithm>

#include "engine/display/display.hpp"

#include "engine/display/sdl_framebuffer_surface_impl.hpp"
#include "util/log.hpp"

namespace pingus {


namespace {

typedef void (*draw_pixel_func)(SDL_Surface* screen, int, int, Color);

inline void draw_pixel16(SDL_Surface* screen, int x, int y, Color c)
{
  Uint32 color = SDL_MapRGBA(screen->format, c.r, c.g, c.b, c.a);

  if (c.a < 255) {
    Uint16 *p;
    unsigned long dp;
    uint8_t alpha;

    // Loses precision for speed
    alpha = static_cast<uint8_t>((255 - c.a) >> 3);

    p = &(static_cast<Uint16 *>(screen->pixels))[x + y * screen->w];
    color = (((color << 16) | color) & 0x07E0F81F);
    dp = *p;
    dp = ((dp << 16) | dp) & 0x07E0F81F;
    dp = ((((dp - color) * alpha) >> 5) + color) & 0x07E0F81F;
    *p = static_cast<Uint16>((dp >> 16) | dp);
  } else {
    static_cast<Uint16*>(screen->pixels)[x + y * screen->w] = static_cast<Uint16>(color);
  }
}

inline void draw_pixel32(SDL_Surface* screen, int x, int y, Color c)
{
  Uint32 color = SDL_MapRGBA(screen->format, c.r, c.g, c.b, c.a);

  if (c.a < 255) {
    Uint32 *p;
    unsigned long sp2;
    unsigned long dp1;
    unsigned long dp2;
    unsigned char alpha;

    alpha = static_cast<unsigned char>(255 - c.a);

    p = &(static_cast<Uint32*>(screen->pixels))[x + y * screen->w];

    sp2 = (color & 0xFF00FF00) >> 8;
    color &= 0x00FF00FF;

    dp1 = *p;
    dp2 = (dp1 & 0xFF00FF00) >> 8;
    dp1 &= 0x00FF00FF;

    dp1 = ((((dp1 - color) * alpha) >> 8) + color) & 0x00FF00FF;
    dp2 = ((((dp2 - sp2) * alpha) >> 8) + sp2) & 0x00FF00FF;
    *p = static_cast<Uint32>(dp1 | (dp2 << 8));
  } else {
    (static_cast<Uint32*>(screen->pixels))[x + y * screen->w] = color;
  }
}

draw_pixel_func get_draw_pixel(SDL_Surface* screen)
{
  switch (screen->format->BitsPerPixel)
  {
    case 16:
      return draw_pixel16;
    case 32:
      return draw_pixel32;
  }
  return NULL;
}

void draw_vline(SDL_Surface* screen, int x, int y, int length, Color color)
{
  draw_pixel_func draw_pixel = get_draw_pixel(screen);
  if (!draw_pixel)
    return;

  SDL_LockSurface(screen);
  for (int i = 0; i < length; ++i) {
    draw_pixel(screen, x, y + i, color);
  }
  SDL_UnlockSurface(screen);
}

void draw_hline(SDL_Surface* screen, int x, int y, int length, Color color)
{
  draw_pixel_func draw_pixel = get_draw_pixel(screen);
  if (!draw_pixel)
    return;

  SDL_LockSurface(screen);
  for (int i = 0; i < length; ++i) {
    draw_pixel(screen, x + i, y, color);
  }
  SDL_UnlockSurface(screen);
}

SDL_Rect Intersection(SDL_Rect* r1, SDL_Rect* r2)
{
  SDL_Rect rect;
  rect.x = std::max(r1->x, r2->x);
  rect.y = std::max(r1->y, r2->y);
  int endx = std::min(r1->x + r1->w, r2->x + r2->w);
  rect.w = std::max(endx - rect.x, 0);
  int endy = std::min(r1->y + r1->h, r2->y + r2->h);
  rect.h = std::max(endy - rect.y, 0);
  return rect;
}

void clip(int& i, int min, int max)
{
  if (i < min)
    i = min;
  else if (i > max)
    i = max;
}

} // namespace


SDLFramebuffer::SDLFramebuffer() :
  m_window(nullptr),
  m_screen(nullptr),
  m_logical_surface(nullptr),
  m_fullscreen(false),
  m_resizable(false),
  cliprect_stack()
{
}

SDLFramebuffer::~SDLFramebuffer()
{
  if (m_logical_surface)
  {
    SDL_FreeSurface(m_logical_surface);
    m_logical_surface = nullptr;
  }
  // m_screen is owned by SDL (SDL_GetWindowSurface), do not free it
  if (m_window)
  {
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }
}

FramebufferSurface
SDLFramebuffer::create_surface(const Surface& surface)
{
  return FramebufferSurface(new SDLFramebufferSurfaceImpl(surface.get_surface()));
}

void
SDLFramebuffer::draw_surface(const FramebufferSurface& surface, Vector2i pos)
{
  SDLFramebufferSurfaceImpl* impl = dynamic_cast<SDLFramebufferSurfaceImpl*>(surface.get_impl());
  SDL_Surface* src = impl->get_surface();

  SDL_Rect dstrect;
  dstrect.x = pos.x;
  dstrect.y = pos.y;
  dstrect.w = src->w;
  dstrect.h = src->h;

  SDL_BlitSurface(src, nullptr, m_logical_surface, &dstrect);
}

void
SDLFramebuffer::draw_surface(const FramebufferSurface& surface, const Rect& srcrect, Vector2i pos)
{
  SDLFramebufferSurfaceImpl* impl = dynamic_cast<SDLFramebufferSurfaceImpl*>(surface.get_impl());
  SDL_Surface* src = impl->get_surface();

  SDL_Rect dstrect;
  dstrect.x = pos.x;
  dstrect.y = pos.y;
  dstrect.w = srcrect.get_width();
  dstrect.h = srcrect.get_height();

  SDL_Rect sdlsrcrect;
  sdlsrcrect.x = srcrect.left;
  sdlsrcrect.y = srcrect.top;
  sdlsrcrect.w = srcrect.get_width();
  sdlsrcrect.h = srcrect.get_height();

  SDL_BlitSurface(src, &sdlsrcrect, m_logical_surface, &dstrect);
}

void
SDLFramebuffer::draw_line(Vector2i pos1, Vector2i pos2, Color color)
{
  int x, y, xlen, ylen, incr;
  int sx = pos1.x;
  int sy = pos1.y;
  int dx = pos2.x;
  int dy = pos2.y;
  draw_pixel_func draw_pixel;
  int clipx1, clipx2, clipy1, clipy2;
  SDL_Rect rect;

  SDL_GetClipRect(m_logical_surface, &rect);
  clipx1 = rect.x;
  clipx2 = rect.x + rect.w - 1;
  clipy1 = rect.y;
  clipy2 = rect.y + rect.h - 1;

  // vertical line
  if (sx == dx) {
    if (sx < clipx1 || sx > clipx2 || (sy < clipy1 && dy < clipy1) || (sy > clipy2 && dy > clipy2)) {
      return;
    }
    clip(sy, clipy1, clipy2);
    clip(dy, clipy1, clipy2);
    if (sy < dy) {
      draw_vline(m_logical_surface, sx, sy, dy - sy + 1, color);
    } else {
      draw_vline(m_logical_surface, dx, dy, sy - dy + 1, color);
    }
    return;
  }

  // horizontal
  if (sy == dy) {
    if (sy < clipy1 || sy > clipy2 || (sx < clipx1 && dx < clipx1) || (sx > clipx2 && dx > clipx2)) {
      return;
    }
    clip(sx, clipx1, clipx2);
    clip(dx, clipx1, clipx2);
    if (sx < dx) {
      draw_hline(m_logical_surface, sx, sy, dx - sx + 1, color);
    } else {
      draw_hline(m_logical_surface, dx, dy, sx - dx + 1, color);
    }
    return;
  }

  draw_pixel = get_draw_pixel(m_logical_surface);
  if (!draw_pixel) {
    return;
  }

  // exchange coordinates
  if (sy > dy) {
    int t = dx;
    dx = sx;
    sx = t;
    t = dy;
    dy = sy;
    sy = t;
  }
  ylen = dy - sy;

  if (sx > dx) {
    xlen = sx - dx;
    incr = -1;
  } else {
    xlen = dx - sx;
    incr = 1;
  }

  y = sy;
  x = sx;

  if (xlen > ylen) {
    if (sx > dx) {
      int t = sx;
      sx = dx;
      dx = t;
      y = dy;
    }

    int p = (ylen << 1) - xlen;

    SDL_LockSurface(m_logical_surface);
    for (x = sx; x < dx; ++x) {
      if (x >= clipx1 && x <= clipx2 && y >= clipy1 && y <= clipy2) {
        draw_pixel(m_logical_surface, x, y, color);
      }
      if (p >= 0) {
        y += incr;
        p += (ylen - xlen) << 1;
      } else {
        p += (ylen << 1);
      }
    }
    SDL_UnlockSurface(m_logical_surface);
    return;
  }

  if (ylen > xlen) {
    int p = (xlen << 1) - ylen;

    SDL_LockSurface(m_logical_surface);
    for (y = sy; y < dy; ++y) {
      if (x >= clipx1 && x <= clipx2 && y >= clipy1 && y <= clipy2) {
        draw_pixel(m_logical_surface, x, y, color);
      }
      if (p >= 0) {
        x += incr;
        p += (xlen - ylen) << 1;
      } else {
        p += (xlen << 1);
      }
    }
    SDL_UnlockSurface(m_logical_surface);
    return;
  }

  // Draw a diagonal line
  if (ylen == xlen) {
    SDL_LockSurface(m_logical_surface);
    while (y != dy) {
      if (x >= clipx1 && x <= clipx2 && y >= clipy1 && y <= clipy2) {
        draw_pixel(m_logical_surface, x, y, color);
      }
      x += incr;
      ++y;
    }
    SDL_UnlockSurface(m_logical_surface);
  }
}

void
SDLFramebuffer::draw_rect(const Rect& rect_, Color color)
{
  Rect rect = rect_;
  rect.normalize();

  draw_line(Vector2i(rect.left,    rect.top),      Vector2i(rect.right-1, rect.top),      color);
  draw_line(Vector2i(rect.left,    rect.bottom-1), Vector2i(rect.right-1, rect.bottom-1), color);
  draw_line(Vector2i(rect.left,    rect.top),      Vector2i(rect.left,    rect.bottom-1), color);
  draw_line(Vector2i(rect.right-1, rect.top),      Vector2i(rect.right-1, rect.bottom-1), color);
}

void
SDLFramebuffer::fill_rect(const Rect& rect_, Color color)
{
  Rect rect = rect_;
  rect.normalize();

  if (color.a == 255)
  {
    SDL_Rect srcrect;

    srcrect.x = rect.left;
    srcrect.y = rect.top;
    srcrect.w = rect.get_width();
    srcrect.h = rect.get_height();

    SDL_FillRect(m_logical_surface, &srcrect, SDL_MapRGBA(m_logical_surface->format, color.r, color.g, color.b, 255));
  }
  else if (color.a != 0)
  {
    int top, bottom, left, right;
    int clipx1, clipx2, clipy1, clipy2;
    SDL_Rect cliprect;

    SDL_GetClipRect(m_logical_surface, &cliprect);
    clipx1 = cliprect.x;
    clipx2 = cliprect.x + cliprect.w - 1;
    clipy1 = cliprect.y;
    clipy2 = cliprect.y + cliprect.h - 1;

    if (rect.right < clipx1 || rect.left > clipx2 || rect.bottom < clipy1 || rect.top > clipy2)
      return;

    top    = rect.top    < clipy1 ? clipy1 : rect.top;
    bottom = rect.bottom > clipy2 ? clipy2 : rect.bottom-1;
    left   = rect.left   < clipx1 ? clipx1 : rect.left;
    right  = rect.right  > clipx2 ? clipx2 : rect.right-1;

    draw_pixel_func draw_pixel = get_draw_pixel(m_logical_surface);
    if (!draw_pixel)
      return;

    SDL_LockSurface(m_logical_surface);
    for (int y = top; y <= bottom; ++y) {
      for (int x = left; x <= right; ++x) {
        draw_pixel(m_logical_surface, x, y, color);
      }
    }
    SDL_UnlockSurface(m_logical_surface);
  }
}

void
SDLFramebuffer::flip()
{
  // Scale the logical 800x600 render target to the physical window surface.
  SDL_BlitScaled(m_logical_surface, nullptr, m_screen, nullptr);
  SDL_UpdateWindowSurface(m_window);
}

void
SDLFramebuffer::update_rects(const std::vector<Rect>& rects)
{
  std::vector<SDL_Rect> sdl_rects;

  for(std::vector<Rect>::const_iterator i = rects.begin(); i != rects.end(); ++i)
  {
    SDL_Rect sdl_rect;
    sdl_rect.x = i->left;
    sdl_rect.y = i->top;
    sdl_rect.w = i->get_width();
    sdl_rect.h = i->get_height();
    sdl_rects.push_back(sdl_rect);
  }

  SDL_UpdateWindowSurfaceRects(m_window,
                                sdl_rects.data(),
                                static_cast<int>(sdl_rects.size()));
}

Size
SDLFramebuffer::get_size() const
{
  int w = 0, h = 0;
  SDL_GetWindowSize(m_window, &w, &h);
  return Size(w, h);
}

void
SDLFramebuffer::set_video_mode(const Size& size, bool fullscreen, bool resizable)
{
  m_fullscreen = fullscreen;
  m_resizable  = resizable;

  if (m_window == nullptr)
  {
    // First-time window creation
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
  }
  else
  {
    // Window already exists — just resize / toggle fullscreen
    if (fullscreen)
      SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
    else
    {
      SDL_SetWindowFullscreen(m_window, 0);
      SDL_SetWindowSize(m_window, size.width, size.height);
    }
  }

  // SDL_GetWindowSurface is invalidated after any resize; always refresh it
  m_screen = SDL_GetWindowSurface(m_window);
  if (m_screen == nullptr)
  {
    log_error("Unable to get window surface: {}", SDL_GetError());
    exit(1);
  }

  // Create an offscreen surface at logical resolution.
  // All drawing targets this surface; it is stretch-blitted to the
  // physical screen on every flip().
  if (m_logical_surface)
    SDL_FreeSurface(m_logical_surface);
  m_logical_surface = SDL_CreateRGBSurface(
    0,
    Display::LOGICAL_WIDTH, Display::LOGICAL_HEIGHT,
    m_screen->format->BitsPerPixel,
    m_screen->format->Rmask, m_screen->format->Gmask,
    m_screen->format->Bmask, m_screen->format->Amask);
  if (m_logical_surface == nullptr)
  {
    log_error("Unable to create logical surface: {}", SDL_GetError());
    exit(1);
  }
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
  SDL_Rect sdl_rect;
  sdl_rect.x = rect.left;
  sdl_rect.y = rect.top;
  sdl_rect.w = rect.get_width();
  sdl_rect.h = rect.get_height();

  if (!cliprect_stack.empty())
  {
    sdl_rect = Intersection(&cliprect_stack.back(), &sdl_rect);
  }

  cliprect_stack.push_back(sdl_rect);
  SDL_SetClipRect(m_logical_surface, &cliprect_stack.back());
}

void
SDLFramebuffer::pop_cliprect()
{
  cliprect_stack.pop_back();
  if (cliprect_stack.empty())
    SDL_SetClipRect(m_logical_surface, nullptr);
  else
    SDL_SetClipRect(m_logical_surface, &cliprect_stack.back());
}


} // namespace pingus

// EOF
