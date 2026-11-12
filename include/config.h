// Compile-time settings. Nothing in this file is secret; credentials live in
// secrets.h, which is git-ignored.
#pragma once

#include <cstddef>
#include <cstdint>

namespace config {

// HUB75 pin map for the Seengreat RGB Matrix HUB75 S3 (seengreat.com/wiki/214).
namespace pins {
constexpr int kR1 = 5;
constexpr int kG1 = 4;
constexpr int kB1 = 6;
constexpr int kR2 = 15;
constexpr int kG2 = 7;
constexpr int kB2 = 17;
constexpr int kA = 8;
constexpr int kB = 18;
constexpr int kC = 10;
constexpr int kD = 9;
constexpr int kE = 16;  // needed for 1/32-scan 64x64 panels
constexpr int kLat = 11;
constexpr int kOe = 13;
constexpr int kClk = 12;
}  // namespace pins

namespace panel {
constexpr int kWidth = 64;
constexpr int kHeight = 64;
constexpr int kPixels = kWidth * kHeight;
}  // namespace panel

namespace display {
constexpr uint8_t kBrightnessDay = 120;  // 0-255
constexpr uint8_t kBrightnessNight = 35;
constexpr int kNightStartHour = 23;  // 11 pm
constexpr int kNightEndHour = 7;     // 7 am
constexpr uint32_t kBrightnessCheckMs = 30000;
constexpr float kGamma = 2.2f;
constexpr int kFadeSteps = 14;
constexpr uint32_t kFadeStepMs = 25;
}  // namespace display

namespace clock {
// POSIX TZ string (US Central). See https://github.com/nayarsystems/posix_tz_db
constexpr const char* kTimezone = "CST6CDT,M3.2.0,M11.1.0";
constexpr const char* kNtpPrimary = "pool.ntp.org";
constexpr const char* kNtpSecondary = "time.nist.gov";
}  // namespace clock

namespace timing {
constexpr uint32_t kPollPlayingMs = 3000;
constexpr uint32_t kPollIdleMs = 6000;
constexpr uint32_t kPollErrorMs = 10000;
constexpr uint32_t kPollWifiDownMs = 2000;
constexpr uint32_t kIdleGraceMs = 15000;                 // keep cover after pause/stop
constexpr uint32_t kIdleBlankMs = 20UL * 60UL * 1000UL;  // blank the clock after this
}  // namespace timing

namespace network {
constexpr uint32_t kWifiConnectTimeoutMs = 20000;
constexpr uint32_t kWifiReconnectAfterMs = 20000;
constexpr uint32_t kTokenRequestTimeoutMs = 10000;
constexpr uint32_t kApiRequestTimeoutMs = 8000;
constexpr size_t kMaxJpegBytes = 32768;  // the 64 px cover is ~3 KB; this is generous
constexpr size_t kMaxJsonBytes = 98304;  // cap for weather / schedule responses
}  // namespace network

// Screens shown in rotation while nothing is playing.
namespace carousel {
constexpr uint32_t kRedrawMs = 1000;
constexpr uint32_t kClockMs = 5000;
constexpr uint32_t kWeatherMs = 5000;
}  // namespace carousel

// Weather from Open-Meteo (free, no API key). Find coordinates for your city at
// https://open-meteo.com/en/docs/geocoding-api
namespace weather {
constexpr const char* kForecastUrl = "https://api.open-meteo.com/v1/forecast";
constexpr float kLatitude = 30.2672f;    // Austin, TX
constexpr float kLongitude = -97.7431f;
constexpr bool kFahrenheit = true;
constexpr uint32_t kRefreshMs = 15UL * 60UL * 1000UL;
constexpr uint32_t kRetryMs = 60UL * 1000UL;
}  // namespace weather

namespace spotify {
constexpr const char* kTokenUrl = "https://accounts.spotify.com/api/token";
constexpr const char* kNowPlayingUrl =
    "https://api.spotify.com/v1/me/player/currently-playing?additional_types=episode";
}  // namespace spotify

}  // namespace config
