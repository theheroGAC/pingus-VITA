// src/engine/display/font.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2005 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "engine/display/font.hpp"

#include <algorithm>
#include <map>
#include <vector>

#include "engine/display/display.hpp"
#include "engine/display/font_description.hpp"
#include "engine/display/framebuffer.hpp"
#include "engine/display/framebuffer_surface.hpp"
#include "util/line_iterator.hpp"
#include "util/log.hpp"
#include "util/utf8.hpp"

#ifdef HAVE_OPENGL
#include "engine/display/opengl/opengl_framebuffer.hpp"
#include "engine/display/opengl/opengl_framebuffer_surface_impl.hpp"
#endif

namespace pingus {

#ifdef HAVE_OPENGL
#ifdef __WII__
#  include <GL/gl.h>
#else
#  include <SDL_opengl.h>
#endif

struct GlyphVertex {
  GLfloat x, y;
  GLfloat u, v;
};

struct GlyphBatch {
  GLuint texture_id;
  std::vector<GlyphVertex> vertices;

  GlyphBatch(GLuint tex_id) : texture_id(tex_id), vertices() {
    vertices.reserve(256);
  }
};

#endif // HAVE_OPENGL

class FontImpl
{
public:
  std::vector<FramebufferSurface> framebuffer_surfaces;
  typedef std::vector<std::unique_ptr<GlyphDescription> > Glyphs;
  Glyphs glyphs;
  int    space_length;
  float  char_spacing;
  float  vertical_spacing;
  int    size;

  FontImpl(const FontDescription& desc) :
    framebuffer_surfaces(),
    glyphs(),
    space_length(),
    char_spacing(desc.char_spacing),
    vertical_spacing(),
    size(desc.size)
  {
    vertical_spacing = static_cast<float>(size) * desc.vertical_spacing;

    glyphs.resize(65536); // 16bit ought to be enough for everybody

    // Copy Unicode -> Glyph mapping
    for(std::vector<GlyphImageDescription>::size_type j = 0; j < desc.images.size(); ++j)
    {
      Surface surface(desc.images[j].pathname);
      if (!surface)
      {
        log_info("IMG: {}", desc.images[j].pathname.str());
        assert(false);
      }

      framebuffer_surfaces.push_back(Display::get_framebuffer()->create_surface(surface));

      for(auto i = desc.images[j].glyphs.begin(); i != desc.images[j].glyphs.end(); ++i)
      {
        if (i->unicode < glyphs.size())
        {
          if (glyphs[i->unicode] == nullptr)
          {
            glyphs[i->unicode] = std::make_unique<GlyphDescription>(*i);
            glyphs[i->unicode]->image = static_cast<int>(framebuffer_surfaces.size()) - 1;
          }
          else
          {
            log_warn("unicode collision on {}", i->unicode);
          }
        }
        else
        {
          log_warn("unicode out of range: {}", i->unicode);
        }
      }
    }
  }

  ~FontImpl()
  {
  }

  void render(Origin origin, int x, int y_, const std::string& text, Framebuffer& fb)
  {
    y_ += get_height();

    float y = float(y_);
    // FIXME: only origins top_left, top_right and top_center do work right now
    LineIterator it(text);
    while(it.next()) {
      render_line(origin, x, int(y), it.get(), fb);
      y += vertical_spacing;
    }
  }

  void render_line(Origin origin, int x, int y, const std::string& text, Framebuffer& fb)
  {
    if (text.empty())
      return;

#ifdef HAVE_OPENGL
    OpenGLFramebuffer* gl_fb = dynamic_cast<OpenGLFramebuffer*>(&fb);
    if (gl_fb)
    {
      render_line_opengl(origin, x, y, text, gl_fb);
      return;
    }
#endif

    render_line_standard(origin, x, y, text, fb);
  }

  void render_line_standard(Origin origin, int x, int y, const std::string& text, Framebuffer& fb)
  {
    Vector2i offset = calc_origin(origin, get_size(text));

    float dstx = float(x - offset.x);
    float dsty = float(y - offset.y);

    UTF8::iterator i(text);
    while(i.next())
    {
      const uint32_t unicode = *i;

      // Skip invalid UTF-8 replacement characters silently
      if (unicode == 0xFFFD)
      {
        continue;
      }

      if (unicode < glyphs.size() && glyphs[unicode])
      {
        const GlyphDescription& glyph = *glyphs[unicode];
        fb.draw_surface(framebuffer_surfaces[glyph.image],
                        glyph.rect, Vector2i(static_cast<int>(dstx), static_cast<int>(dsty)) + glyph.offset);
        dstx += static_cast<float>(glyph.advance) + char_spacing;
      }
    }
  }

#ifdef HAVE_OPENGL
  void render_line_opengl(Origin origin, int x, int y, const std::string& text, OpenGLFramebuffer* gl_fb)
  {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    Vector2i offset = calc_origin(origin, get_size(text));

    int base_x = x - offset.x;
    int base_y = y - offset.y;

    std::map<GLuint, GlyphBatch> batches;
    float dstx = 0.0f;

    UTF8::iterator i(text);
    while(i.next())
    {
      const uint32_t unicode = *i;

      if (unicode == 0xFFFD)
        continue;

      if (unicode < glyphs.size() && glyphs[unicode])
      {
        const GlyphDescription& glyph = *glyphs[unicode];

        OpenGLFramebufferSurfaceImpl* gl_impl =
          dynamic_cast<OpenGLFramebufferSurfaceImpl*>(framebuffer_surfaces[glyph.image].get_impl());

        if (!gl_impl)
        {
          dstx += static_cast<float>(glyph.advance) + char_spacing;
          continue;
        }

        const std::vector<OpenGLTile>& tiles = gl_impl->get_tiles();

        for (const OpenGLTile& tile : tiles)
        {
          if (glyph.rect.left >= tile.rect.left &&
              glyph.rect.right <= tile.rect.right &&
              glyph.rect.top >= tile.rect.top &&
              glyph.rect.bottom <= tile.rect.bottom)
          {
            auto [batch_it, inserted] = batches.emplace(
              std::piecewise_construct,
              std::forward_as_tuple(tile.handle),
              std::forward_as_tuple(tile.handle));
            (void)inserted;

            GlyphBatch& batch = batch_it->second;

            float glyph_left_in_tile = static_cast<float>(glyph.rect.left - tile.rect.left);
            float glyph_top_in_tile = static_cast<float>(glyph.rect.top - tile.rect.top);
            float glyph_width = static_cast<float>(glyph.rect.get_width());
            float glyph_height = static_cast<float>(glyph.rect.get_height());

            float tx1 = glyph_left_in_tile * tile.u_scale;
            float ty1 = glyph_top_in_tile * tile.v_scale;
            float tx2 = (glyph_left_in_tile + glyph_width) * tile.u_scale;
            float ty2 = (glyph_top_in_tile + glyph_height) * tile.v_scale;

            float screen_x = static_cast<float>(base_x) + dstx + static_cast<float>(glyph.offset.x);
            float screen_y = static_cast<float>(base_y) + static_cast<float>(glyph.offset.y);

            batch.vertices.push_back({screen_x, screen_y + glyph_height, tx1, ty2});
            batch.vertices.push_back({screen_x + glyph_width, screen_y + glyph_height, tx2, ty2});
            batch.vertices.push_back({screen_x + glyph_width, screen_y, tx2, ty1});
            batch.vertices.push_back({screen_x, screen_y, tx1, ty1});

            break;
          }
        }

        dstx += static_cast<float>(glyph.advance) + char_spacing;
      }
    }

    if (!batches.empty())
    {
      for (auto& batch_pair : batches)
      {
        GlyphBatch& batch = batch_pair.second;

        if (batch.vertices.empty())
          continue;

        glBindTexture(GL_TEXTURE_2D, batch.texture_id);

        glVertexPointer(2, GL_FLOAT, sizeof(GlyphVertex), &batch.vertices[0].x);
        glTexCoordPointer(2, GL_FLOAT, sizeof(GlyphVertex), &batch.vertices[0].u);

        glDrawArrays(GL_QUADS, 0, batch.vertices.size());
      }
    }

    gl_fb->invalidate_state();
  }
#endif // HAVE_OPENGL

  int get_height() const
  {
    return size;
  }

  float get_width(uint32_t unicode) const
  {
    if (unicode < glyphs.size() && glyphs[unicode])
      return static_cast<float>(glyphs[unicode]->advance);
    else
      return 0;
  }

  float get_width(const std::string& text) const
  {
    float width = 0.0f;
    float last_width = 0;

    UTF8::iterator i(text);
    while(i.next())
    {
      const uint32_t unicode = *i;

      if (unicode == '\n')
      {
        last_width = std::max(last_width, width);
        width = 0;
      }
      else if (unicode != 0xFFFD)
      {
        width += get_width(unicode) + char_spacing;
      }
    }

    return std::max(width, last_width);
  }

  Size get_size(const std::string& text) const
  {
    return Size(static_cast<int>(get_width(text)), get_height());
  }

  Rect bounding_rect(int x, int y, const std::string& str) const
  {
    return Rect(Vector2i(x, y), get_size(str));
  }
};

Font::Font() :
  impl()
{
}

Font::Font(const FontDescription& desc) :
  impl(new FontImpl(desc))
{
}

void
Font::render(int x, int y, const std::string& text, Framebuffer& fb)
{
  if (impl)
    impl->render(origin_top_left, x, y, text, fb);
}

void
Font::render(Origin origin, int x, int y, const std::string& text, Framebuffer& fb)
{
  if (impl)
    impl->render(origin, x, y, text, fb);
}

int
Font::get_height() const
{
  if (impl)
    return impl->get_height();
  else
    return 0;
}

float
Font::get_width(uint32_t unicode) const
{
  if (impl)
    return impl->get_width(unicode);
  else
    return 0;
}

float
Font::get_width(const std::string& text) const
{
  if (impl)
    return impl->get_width(text);
  else
    return 0;
}

Size
Font::get_size(const std::string& str) const
{
  if (impl)
    return impl->get_size(str);
  else
    return Size();
}

Rect
Font::bounding_rect(int x, int y, const std::string& str) const
{
  if (impl)
    return impl->bounding_rect(x, y, str);
  else
    return Rect();
}


} // namespace pingus

// EOF
