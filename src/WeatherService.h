#pragma once

#include <ledcore/Weather.h>

#include "PeriodicSource.h"

struct WeatherData {
  int tempNow = 0;
  int tempHigh = 0;
  int tempLow = 0;
  ledcore::WeatherIcon icon = ledcore::WeatherIcon::Cloudy;
};

// Current conditions and today's high/low from Open-Meteo.
class WeatherService : public PeriodicSource {
 public:
  WeatherService();

  const WeatherData& data() const { return data_; }

 protected:
  bool fetch() override;

 private:
  WeatherData data_;
};
