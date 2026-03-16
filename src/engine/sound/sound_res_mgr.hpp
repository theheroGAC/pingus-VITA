// src/engine/sound/sound_res_mgr.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2002 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_SOUND_SOUND_RES_MGR_HPP
#define HEADER_PINGUS_ENGINE_SOUND_SOUND_RES_MGR_HPP

#include <unordered_map>
#include <string>
#include <string_view>
#include <memory>
#include <SDL2/SDL_mixer.h>

namespace pingus {


typedef Mix_Chunk* SoundHandle;

// Custom deleter for Mix_Chunk to work with unique_ptr
struct MixChunkDeleter {
  void operator()(Mix_Chunk* chunk) const {
    if (chunk) Mix_FreeChunk(chunk);
  }
};

using SoundPtr = std::unique_ptr<Mix_Chunk, MixChunkDeleter>;

class SoundResMgr
{
private:
  // Transparent hashing allows looking up std::string keys using std::string_view
  struct StringHash {
    using is_transparent = void;
    size_t operator()(std::string_view sv) const {
      return std::hash<std::string_view>{}(sv);
    }
  };

  typedef std::unordered_map<std::string, SoundPtr, StringHash, std::equal_to<>> SoundMap;
  static SoundMap sound_map;

public:
  static SoundHandle load(std::string_view name);
  static void free_sound_map();
private:
  SoundResMgr (const SoundResMgr&);
  SoundResMgr& operator= (const SoundResMgr&);
};


} // namespace pingus
#endif

// EOF
