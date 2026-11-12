// Maps WMO weather codes (as returned by Open-Meteo) to a small set of icons.
#pragma once

namespace ledcore {

enum class WeatherIcon { ClearDay, ClearNight, PartlyCloudy, Cloudy, Fog, Rain, Snow, Thunder };

inline WeatherIcon iconForWmoCode(int code, bool isDay) {
  if (code == 0 || code == 1) return isDay ? WeatherIcon::ClearDay : WeatherIcon::ClearNight;
  if (code == 2) return isDay ? WeatherIcon::PartlyCloudy : WeatherIcon::Cloudy;
  if (code == 3) return WeatherIcon::Cloudy;
  if (code == 45 || code == 48) return WeatherIcon::Fog;
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return WeatherIcon::Rain;
  if ((code >= 71 && code <= 77) || code == 85 || code == 86) return WeatherIcon::Snow;
  if (code >= 95 && code <= 99) return WeatherIcon::Thunder;
  return WeatherIcon::Cloudy;  // unknown codes: a neutral icon beats a wrong one
}

}  // namespace ledcore
