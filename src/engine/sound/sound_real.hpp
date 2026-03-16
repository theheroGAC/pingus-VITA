// src/engine/sound/sound_real.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 2000 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_ENGINE_SOUND_SOUND_REAL_HPP
#define HEADER_PINGUS_ENGINE_SOUND_SOUND_REAL_HPP

#include <SDL2/SDL_mixer.h>
#include <string_view>

#include "engine/sound/sound_impl.hpp"

namespace pingus::sound {

/** A simple wrapper class around SDL_Mixer, it will init itself
    automatically if a sound is played. */
class PingusSoundReal : public PingusSoundImpl
{
private:
  /** The current music file */
  Mix_Music* music_sample;

  float m_music_volume;
  float m_sound_volume;
  float m_master_volume;

public:
  PingusSoundReal ();
  virtual ~PingusSoundReal ();

  /** Load a music file and play it immediately.
      @param filename The complete filename
      @param volume   The volume to play the music with
      @param loop     The music file should loop continuously  */
  virtual void real_play_music(std::string_view filename, float volume, bool loop) override;

  virtual void real_stop_music() override;

  /** Load a sound file and play it immediately
      @param filename The complete filename
      @param volume   The volume to play the sound at
      @param panning  The panning to play the sound with */
  virtual void real_play_sound(std::string_view filename, float volume, float panning) override;

  virtual void set_sound_volume(float volume) override;
  virtual void set_music_volume(float volume) override;
  virtual void set_master_volume(float volume) override;

  virtual float get_sound_volume() const override;
  virtual float get_music_volume() const override;
  virtual float get_master_volume() const override;

private:
  void apply_volume_changes();

private:
  PingusSoundReal (const PingusSoundReal&);
  PingusSoundReal& operator= (const PingusSoundReal&);
};

} // namespace pingus::sound

#endif

// EOF
