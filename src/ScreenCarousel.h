#pragma once

#include <Arduino.h>

#include "screens/Screen.h"

// Rotates through screens, skipping any that have nothing to show.
class ScreenCarousel {
 public:
  static constexpr size_t kMaxScreens = 8;

  ScreenCarousel(Screen* const* screens, size_t count) : screens_(screens), count_(count) {}

  // Start again from the first screen on the next tick.
  void reset() { started_ = false; }

  // Advances if the current screen's time is up, then draws the current screen.
  void tick(Display& display, uint32_t nowMs);

 private:
  Screen* const* screens_;
  size_t count_;
  size_t index_ = 0;
  uint32_t shownAtMs_ = 0;
  bool started_ = false;
};
