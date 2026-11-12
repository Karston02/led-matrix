#pragma once

#include <Arduino.h>

#include "Display.h"

// One page of the idle-time carousel. A screen only knows how to draw itself and
// whether it currently has something worth showing; data fetching lives elsewhere.
class Screen {
 public:
  virtual ~Screen() = default;

  virtual const char* name() const = 0;
  virtual uint32_t durationMs() const = 0;

  // False when the screen has no data yet; the carousel skips it.
  virtual bool isAvailable(uint32_t nowMs) const = 0;

  virtual void draw(Display& display, uint32_t nowMs) = 0;
};
