#pragma once

#include <cstdint>
#include <cstring>

#include "config.h"

// One full-panel image in RGB565 (2 bytes per pixel, 8 KB for 64x64).
struct Frame {
  uint16_t px[config::panel::kPixels];

  void clear() { std::memset(px, 0, sizeof(px)); }
};
