#include "WeatherService.h"

#include <ArduinoJson.h>

#include <cmath>

#include "HttpJson.h"
#include "config.h"

WeatherService::WeatherService()
    : PeriodicSource("weather", config::weather::kRefreshMs, config::weather::kRetryMs) {}

bool WeatherService::fetch() {
  const String url = String(config::weather::kForecastUrl) +
                     "?latitude=" + String(config::weather::kLatitude, 4) +
                     "&longitude=" + String(config::weather::kLongitude, 4) +
                     "&current=temperature_2m,weather_code,is_day" +
                     "&daily=temperature_2m_max,temperature_2m_min" +
                     "&temperature_unit=" +
                     (config::weather::kFahrenheit ? "fahrenheit" : "celsius") +
                     "&timezone=auto&forecast_days=1";

  JsonDocument filter;
  filter["current"]["temperature_2m"] = true;
  filter["current"]["weather_code"] = true;
  filter["current"]["is_day"] = true;
  filter["daily"]["temperature_2m_max"][0] = true;
  filter["daily"]["temperature_2m_min"][0] = true;

  JsonDocument doc;
  if (!http_json::get(url, filter, doc)) {
    return false;
  }

  JsonVariant current = doc["current"];
  JsonVariant daily = doc["daily"];
  if (current["temperature_2m"].isNull() || daily["temperature_2m_max"][0].isNull() ||
      daily["temperature_2m_min"][0].isNull()) {
    return false;
  }

  WeatherData next;
  next.tempNow = static_cast<int>(std::lround(current["temperature_2m"].as<float>()));
  next.tempHigh = static_cast<int>(std::lround(daily["temperature_2m_max"][0].as<float>()));
  next.tempLow = static_cast<int>(std::lround(daily["temperature_2m_min"][0].as<float>()));
  next.icon = ledcore::iconForWmoCode(current["weather_code"] | 3, (current["is_day"] | 1) != 0);
  data_ = next;
  return true;
}
