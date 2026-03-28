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

#include <cmath> // Required for std::fabs, std::sqrt, std::atan2, std::isfinite
#include "engine/display/display.hpp"
#include "engine/display/sdl_framebuffer_surface_impl.hpp"
#include "util/log.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace pingus {

SDLFramebuffer::SDLFramebuffer() :
  m_window(nullptr),
  m_renderer(nullptr),
  m_pixel_texture(nullptr),
  m_fullscreen(false),
  m_resizable(false),
  cliprect_stack()
{
}

SDLFramebuffer::~SDLFramebuffer()
{
  if (m_pixel_texture)
  {
    SDL_DestroyTexture(m_pixel_texture);
    m_pixel_texture = nullptr;
  }
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

  // Delegate to the srcrect overload with a full-image source rectangle so
  // that geometry clipping and UV precision handling are applied consistently
  // for all draw calls regardless of which overload the caller uses.
  draw_surface(surface, Rect(0, 0, impl->get_width(), impl->get_height()), pos);
}

void
SDLFramebuffer::draw_surface(const FramebufferSurface& surface, const Rect& srcrect, Vector2i pos)
{
  SDLFramebufferSurfaceImpl* impl = dynamic_cast<SDLFramebufferSurfaceImpl*>(surface.get_impl());

  float src_x = (float)srcrect.left;
  float src_y = (float)srcrect.top;
  float src_w = (float)srcrect.get_width();
  float src_h = (float)srcrect.get_height();

  float dst_x = (float)pos.x;
  float dst_y = (float)pos.y;
  float dst_w = src_w;
  float dst_h = src_h;

  // Software-clip the destination rectangle to the logical screen bounds
  // before submitting geometry to the renderer.
  //
  // On hardware renderers backed by fixed-point GPU geometry pipelines (such
  // as the Wii's GX), passing vertex coordinates that extend far outside the
  // physical viewport can cause arithmetic overflow in the hardware clipper,
  // producing corrupted geometry that renders as large coloured triangles or
  // texture-coordinate tears persisting on screen.  Desktop GPU drivers
  // handle extreme off-screen coordinates gracefully with floating-point
  // math, so the problem only manifests on the target hardware.
  //
  // The clip boundary is fixed at the logical screen extents rather than
  // derived from the active hardware scissor rect.  Using the scissor rect
  // would cause the software clip to change dimensions frame-to-frame during
  // screen transitions (where the scissor rect animates), which shifts the
  // computed source offset and produces visible background jitter.  The
  // hardware scissor continues to handle content masking independently.
  const float cl = 0.0f;
  const float ct = 0.0f;
  const float cr = (float)Display::LOGICAL_WIDTH;
  const float cb = (float)Display::LOGICAL_HEIGHT;

  if (dst_x >= cr || dst_y >= cb || dst_x + dst_w <= cl || dst_y + dst_h <= ct)
    return;

  if (dst_x < cl) { float d = cl-dst_x; src_x+=d; src_w-=d; dst_w-=d; dst_x=cl; }
  if (dst_y < ct) { float d = ct-dst_y; src_y+=d; src_h-=d; dst_h-=d; dst_y=ct; }
  if (dst_x+dst_w > cr) { float d=(dst_x+dst_w)-cr; src_w-=d; dst_w-=d; }
  if (dst_y+dst_h > cb) { float d=(dst_y+dst_h)-cb; src_h-=d; dst_h-=d; }

  if (src_w <= 0.0f || src_h <= 0.0f || dst_w <= 0.0f || dst_h <= 0.0f)
    return;

  // Compute UV coordinates as normalised floats derived from the original
  // (pre-downscale) pixel dimensions.  The downscale factor sx cancels out:
  //   u = src_x * sx / (m_width * sx) = src_x / m_width
  // This keeps UV values continuous as src_x changes by fractional amounts,
  // avoiding the texel-step quantisation that would occur if the source rect
  // were first converted to integer texture-pixel coordinates via SDL_Rect.
  // SDL_RenderGeometry forwards these float UVs directly to the GPU without
  // further integer truncation.
  //
  // Apply a 0.5-texel inset on all four sides of the source rect.  This is
  // the same technique used by the OpenGL backend (see opengl_framebuffer.cpp)
  // and is necessary for two reasons:
  //
  //  1. Sprite sheets – each animation frame occupies a sub-rectangle of the
  //     texture.  Without an inset, bilinear filtering at the frame edge can
  //     sample 0.5 texels past the boundary into an adjacent frame.  The
  //     alpha-bleed preprocessing (sdl_framebuffer_surface_impl.cpp) fills
  //     those boundary texels with the nearest opaque colour of the *current*
  //     frame, but the 4-axis-aligned passes can also propagate colour from a
  //     neighbouring frame into the boundary texel of the current frame.  The
  //     inset ensures the GPU never reaches that contaminated texel.
  //
  //  2. Tiled backgrounds – when the same surface is drawn in a grid (e.g.
  //     SurfaceBackground), the edge texels of a tile are sampled at UV ≈ 1.
  //     On some SDL/GPU backends bilinear interpolation at the exact texture
  //     edge blends with the border colour (often transparent black), producing
  //     a dark fringe at every tile seam.  The inset keeps the sample point
  //     at the edge texel's centre, where the alpha-bled colour is guaranteed
  //     correct regardless of the border-colour mode.
  const float iw = 1.0f / (float)impl->get_width();
  const float ih = 1.0f / (float)impl->get_height();
  const float u0 = (src_x        + 0.5f) * iw,  v0 = (src_y        + 0.5f) * ih;
  const float u1 = (src_x+src_w  - 0.5f) * iw,  v1 = (src_y+src_h  - 0.5f) * ih;

  SDL_Vertex verts[4] = {
    {{dst_x,       dst_y      }, {255,255,255,255}, {u0, v0}},
    {{dst_x+dst_w, dst_y      }, {255,255,255,255}, {u1, v0}},
    {{dst_x+dst_w, dst_y+dst_h}, {255,255,255,255}, {u1, v1}},
    {{dst_x,       dst_y+dst_h}, {255,255,255,255}, {u0, v1}},
  };
  static const int idx[] = {0, 1, 2, 0, 2, 3};
  SDL_RenderGeometry(m_renderer, impl->get_texture(), verts, 4, idx, 6);
}

void
SDLFramebuffer::draw_line(Vector2i pos1, Vector2i pos2, Color color)
{
  // The SDL_RenderDrawLine function is fundamentally broken in the SDL-OGC
  // backend for the Wii, causing GPU crashes. We must avoid it entirely.
  //
  // This implementation bypasses it by drawing a 1x1 white pixel texture,
  // stretched and rotated to form a line. This uses the texture rendering
  // path (SDL_RenderCopyEx), which is stable on the hardware.

  // Trivial reject if the line has no length
  if (pos1.x == pos2.x && pos1.y == pos2.y) {
    return;
  }

  float x1 = (float)pos1.x;
  float y1 = (float)pos1.y;
  float x2 = (float)pos2.x;
  float y2 = (float)pos2.y;

  float dx = x2 - x1;
  float dy = y2 - y1;
  float length = std::sqrt(dx*dx + dy*dy);
  float angle = std::atan2(dy, dx) * (180.0f / M_PI);

  SDL_Rect dst_rect = { static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(length), 1 };
  SDL_Point center = { 0, 0 }; // Rotate around the starting point of the line

  SDL_SetTextureColorMod(m_pixel_texture, color.r, color.g, color.b);
  SDL_SetTextureAlphaMod(m_pixel_texture, color.a);
  SDL_SetRenderDrawBlendMode(m_renderer,
    color.a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);

  SDL_RenderCopyEx(m_renderer, m_pixel_texture, NULL, &dst_rect, angle, &center, SDL_FLIP_NONE);
}

void
SDLFramebuffer::draw_rect(const Rect& rect_, Color color)
{
  Rect r = rect_;
  r.normalize();

  // Route through draw_line rather than SDL_RenderDrawLines or four direct
  // SDL_RenderDrawLine calls.  SDL_RenderDrawLines clips each segment endpoint
  // independently when a point lies off-screen, but the implicit "current
  // position" between consecutive segments does not reset between calls.  The
  // closing segment from the last point back to the first therefore connects
  // two independently-clipped edge positions, producing a spurious diagonal
  // line across the screen when the rectangle extends outside the viewport.
  // draw_line additionally avoids SDL_RenderDrawLine entirely, which is broken
  // in the SDL-OGC backend and causes GPU crashes on the Wii hardware.
  draw_line(Vector2i(r.left, r.top),    Vector2i(r.right, r.top),    color);
  draw_line(Vector2i(r.left, r.bottom), Vector2i(r.right, r.bottom), color);
  draw_line(Vector2i(r.left, r.top),    Vector2i(r.left, r.bottom),  color);
  draw_line(Vector2i(r.right, r.top),   Vector2i(r.right, r.bottom), color);
}

void
SDLFramebuffer::fill_rect(const Rect& rect_, Color color)
{
  // Reject corrupted geometry from physics calculations (NaN/Infinity)
  if (!std::isfinite(rect_.left) || !std::isfinite(rect_.top) ||
      !std::isfinite(rect_.right) || !std::isfinite(rect_.bottom))
  {
    return;
  }

  Rect r = rect_;
  r.normalize();

  // Software-clip to logical screen bounds to protect hardware rasterizers
  const int cl = 0;
  const int ct = 0;
  const int cr = Display::LOGICAL_WIDTH;
  const int cb = Display::LOGICAL_HEIGHT;

  if (r.left >= cr || r.top >= cb || r.right <= cl || r.bottom <= ct)
    return;

  if (r.left < cl) r.left = cl;
  if (r.top < ct) r.top = ct;
  if (r.right > cr) r.right = cr;
  if (r.bottom > cb) r.bottom = cb;

  if (r.get_width() <= 0 || r.get_height() <= 0)
    return;

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

  // Clear the back buffer after presenting so the next frame starts from a
  // known blank state.  SDL_Renderer does not automatically clear between
  // frames; without this, any screen region not redrawn each frame retains
  // its previous content, causing ghost trails from moving elements.
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

    // Create the 1x1 pixel texture for drawing lines
    m_pixel_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 1, 1);
    if (m_pixel_texture) {
        Uint32 white_pixel = 0xFFFFFFFF;
        SDL_UpdateTexture(m_pixel_texture, NULL, &white_pixel, 4);
        SDL_SetTextureBlendMode(m_pixel_texture, SDL_BLENDMODE_BLEND);
    } else {
        log_error("Unable to create pixel texture: {}", SDL_GetError());
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
