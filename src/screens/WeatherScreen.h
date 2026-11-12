#pragma once

#include "Screen.h"
#include "WeatherService.h"

class WeatherScreen : public Screen {
 public:
  explicit WeatherScreen(const WeatherService& weather) : weather_(weather) {}

  const char* name() const override { return "weather"; }
  uint32_t durationMs() const override;
  bool isAvailable(uint32_t nowMs) const override { return weather_.hasRecentData(nowMs); }
  void draw(Display& display, uint32_t nowMs) override;

 private:
  const WeatherService& weather_;
};
