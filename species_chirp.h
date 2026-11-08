#pragma once

#include <stdint.h>

// Own synthesized species chirps. These profiles contain no
// recorded or ROM-derived Pokemon audio data.
enum ChirpWave : uint8_t {
  CHIRP_SQUARE = 0,
  CHIRP_TRI,
  CHIRP_SOFT,
  CHIRP_NOISE,
};

struct SpeciesChirpNote {
  uint16_t frequency;
  uint16_t durationMs;
  int16_t slide;
  uint8_t volume;
  uint8_t wave;
};

struct SpeciesChirpProfile {
  SpeciesChirpNote notes[4];
  uint8_t count;
};

// Returns its own melodic synthesized call for every included dex number.
// Invalid dex numbers return false and never produce audio.
bool speciesChirpProfile(int16_t dex, SpeciesChirpProfile *out);
