// Colour helpers: RGB565 unpacking, channel blending and gamma correction.
// Pure functions with no hardware dependencies.
#pragma once

#include <cmath>
#include <cstdint>

namespace ledcore {

struct Rgb {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

// Expands an RGB565 pixel to 8 bits per channel, replicating the high bits into
// the low bits so that 0x1F -> 255 and 0x00 -> 0 exactly.
inline Rgb unpack565(uint16_t c) {
  const uint8_t r5 = (c >> 11) & 0x1F;
  const uint8_t g6 = (c >> 5) & 0x3F;
  const uint8_t b5 = c & 0x1F;
  return Rgb{static_cast<uint8_t>((r5 << 3) | (r5 >> 2)),
             static_cast<uint8_t>((g6 << 2) | (g6 >> 4)),
             static_cast<uint8_t>((b5 << 3) | (b5 >> 2))};
}

// Linear blend of one channel. mix = 0 returns `from`, mix = 255 returns `to`.
inline uint8_t blendChannel(uint8_t from, uint8_t to, uint8_t mix) {
  const int inverse = 255 - mix;
  return static_cast<uint8_t>((from * inverse + to * mix) / 255);
}

inline Rgb blend(Rgb from, Rgb to, uint8_t mix) {
  return Rgb{blendChannel(from.r, to.r, mix), blendChannel(from.g, to.g, mix),
             blendChannel(from.b, to.b, mix)};
}

// Fills a 256-entry lookup table that maps linear 8-bit values to
// gamma-corrected ones. LED panels are linear emitters, so without this
// correction mid-tones look washed out.
inline void buildGammaLut(uint8_t lut[256], float gamma) {
  for (int i = 0; i < 256; i++) {
    const float corrected = std::pow(static_cast<float>(i) / 255.0f, gamma);
    lut[i] = static_cast<uint8_t>(corrected * 255.0f + 0.5f);
  }
}

}  // namespace ledcore
