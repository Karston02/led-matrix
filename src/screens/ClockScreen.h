#pragma once

#include "Screen.h"
#include "TimeService.h"

class ClockScreen : public Screen {
 public:
  explicit ClockScreen(const TimeService& time) : time_(time) {}

  const char* name() const override { return "clock"; }
  uint32_t durationMs() const override;
  bool isAvailable(uint32_t) const override { return true; }  // always shows something
  void draw(Display& display, uint32_t nowMs) override;

 private:
  const TimeService& time_;
};
