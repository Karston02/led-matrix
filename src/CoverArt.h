#pragma once

#include <Arduino.h>

#include "Frame.h"

namespace cover_art {

// Downloads a JPEG cover from `url` and decodes it into `out` (RGB565, cropped
// or zero-padded to the panel size). Returns false on any network or decode error.
bool fetch(const String& url, Frame& out);

}  // namespace cover_art
