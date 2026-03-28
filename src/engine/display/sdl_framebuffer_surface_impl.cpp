// src/engine/display/sdl_framebuffer_surface_impl.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "engine/display/sdl_framebuffer_surface_impl.hpp"

#include <algorithm>
#include <stdexcept>

#include "util/log.hpp"

namespace pingus {

SDLFramebufferSurfaceImpl::SDLFramebufferSurfaceImpl(SDL_Renderer* renderer, SDL_Surface* src) :
  m_texture(nullptr),
  m_width(src->w),
  m_height(src->h),
  m_scale_x(1.0f),
  m_scale_y(1.0f)
{
  // Use the source surface directly -- letting SDL choose the optimal internal
  // format avoids the byte-order issues that caused JPEGs to become transparent
  // when manually converted to ARGB8888 on some SDL2 ports.
  SDL_Surface* upload = src;
  SDL_Surface* scaled  = nullptr;

  // On Wii, GX caps textures at 1024 in any dimension.  Large backgrounds
  // (e.g. a 1920x1200 worldmap layer) must be scaled before upload.
  //
  // IMPORTANT: m_width / m_height stay at the ORIGINAL dimensions.
  // The game uses these for sprite frame sizes, worldmap scroll offsets, etc.
  // We only store scale factors so draw_surface can translate source rects
  // to the actual (smaller) texture coordinates.
  SDL_RendererInfo info = {};
  if (SDL_GetRendererInfo(renderer, &info) == 0 &&
      info.max_texture_width  > 0 &&
      info.max_texture_height > 0 &&
      (src->w > info.max_texture_width || src->h > info.max_texture_height))
  {
    const float scale = std::min(
      static_cast<float>(info.max_texture_width)  / src->w,
      static_cast<float>(info.max_texture_height) / src->h);

    const int newW = std::max(1, static_cast<int>(src->w * scale));
    const int newH = std::max(1, static_cast<int>(src->h * scale));

    log_info("SDLFramebufferSurfaceImpl: downscaling {}x{} -> {}x{} (hw limit {}x{})",
             src->w, src->h, newW, newH,
             info.max_texture_width, info.max_texture_height);

    scaled = SDL_CreateRGBSurfaceWithFormat(
      0, newW, newH, src->format->BitsPerPixel, src->format->format);
    if (scaled)
    {
      SDL_SetSurfaceBlendMode(src, SDL_BLENDMODE_NONE);
      SDL_BlitScaled(src, nullptr, scaled, nullptr);
      upload    = scaled;
      // Scale factors: multiply a source-rect coordinate by these to get
      // the corresponding coordinate in the actual (smaller) texture.
      m_scale_x = static_cast<float>(newW) / src->w;
      m_scale_y = static_cast<float>(newH) / src->h;
      // m_width / m_height intentionally left at original values.
    }
  }

  // Determine if we need to apply alpha bleeding to prevent dark fringes
  // during bilinear filtering (scaling).
  Uint32 colorkey_val = 0;
  const bool has_colorkey = (SDL_GetColorKey(src, &colorkey_val) == 0);
  const bool has_alpha    = (src->format->Amask != 0) || has_colorkey;

  if (has_alpha)
  {
    // Convert to a temporary 32-bit RGBA surface for bleeding. This ensures we
    // have a known pixel format (4 bpp) and can safely manipulate RGB values
    // of fully transparent pixels.
    SDL_Surface* bled = SDL_CreateRGBSurfaceWithFormat(0, upload->w, upload->h, 32, SDL_PIXELFORMAT_RGBA32);
    if (bled)
    {
      SDL_SetSurfaceBlendMode(upload, SDL_BLENDMODE_NONE);
      SDL_BlitSurface(upload, nullptr, bled, nullptr);

      // Alpha bleed: fill transparent pixels with the colour of their nearest
      // opaque neighbour. Colorkey blits leave transparent pixels as (0,0,0,0),
      // so bilinear filtering at non-native scales blends opaque content with
      // transparent black, producing a dark fringe along every transparency
      // boundary and bleeding adjacent frames in sprite sheets.
      // Four axis-aligned passes propagate opaque colours into all transparent
      // regions in O(w*h) time without allocating additional memory.
      SDL_LockSurface(bled);
      Uint8* px    = static_cast<Uint8*>(bled->pixels);
      int    bpp   = bled->format->BytesPerPixel;
      int    pitch = bled->pitch;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
      int    a_off = 3 - (bled->format->Ashift / 8);
#else
      int    a_off = bled->format->Ashift / 8;
#endif

      auto get_alpha = [&](int x, int y) -> Uint8 {
        return px[y * pitch + x * bpp + a_off];
      };
      auto copy_rgb = [&](int dx, int dy, int sx, int sy) {
        Uint8*       d = px + dy * pitch + dx * bpp;
        const Uint8* s = px + sy * pitch + sx * bpp;
        for (int b = 0; b < bpp; ++b)
          if (b != a_off) d[b] = s[b];
      };

      int W = bled->w;
      int H = bled->h;

      for (int ty = 1;   ty < H;  ++ty) for (int tx = 0;   tx < W;  ++tx) if (get_alpha(tx,ty)==0 && get_alpha(tx,ty-1)>0) copy_rgb(tx,ty,tx,ty-1); // top-down
      for (int ty = H-2; ty >= 0; --ty) for (int tx = 0;   tx < W;  ++tx) if (get_alpha(tx,ty)==0 && get_alpha(tx,ty+1)>0) copy_rgb(tx,ty,tx,ty+1); // bottom-up
      for (int ty = 0;   ty < H;  ++ty) for (int tx = 1;   tx < W;  ++tx) if (get_alpha(tx,ty)==0 && get_alpha(tx-1,ty)>0) copy_rgb(tx,ty,tx-1,ty); // left-right
      for (int ty = 0;   ty < H;  ++ty) for (int tx = W-2; tx >= 0; --tx) if (get_alpha(tx,ty)==0 && get_alpha(tx+1,ty)>0) copy_rgb(tx,ty,tx+1,ty); // right-left

      SDL_UnlockSurface(bled);
      m_texture = SDL_CreateTextureFromSurface(renderer, bled);
      SDL_FreeSurface(bled);
    }
  }

  // Fallback if bleeding was skipped or failed
  if (!m_texture)
  {
    m_texture = SDL_CreateTextureFromSurface(renderer, upload);
  }

  if (scaled)
    SDL_FreeSurface(scaled);

  if (!m_texture)
  {
    throw std::runtime_error(
      std::string("SDLFramebufferSurfaceImpl: SDL_CreateTextureFromSurface failed: ") +
      SDL_GetError());
  }

  if (has_alpha)
    SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
}

SDLFramebufferSurfaceImpl::~SDLFramebufferSurfaceImpl()
{
  if (m_texture)
  {
    SDL_DestroyTexture(m_texture);
    m_texture = nullptr;
  }
}

} // namespace pingus

// EOF
