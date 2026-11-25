#pragma once

#include <Arduino.h>
#include <ledcore/PlaybackPolicy.h>

#include "Credentials.h"
#include "Display.h"
#include "Frame.h"
#include "PeriodicSource.h"
#include "ScreenCarousel.h"
#include "SpotifyAuth.h"
#include "SpotifyClient.h"
#include "TimeService.h"
#include "WeatherService.h"
#include "WifiManager.h"
#include "screens/ClockScreen.h"
#include "screens/WeatherScreen.h"

// One Spotify login: its token keeper and the client that uses it.
struct SpotifyAccount {
  SpotifyAccount(const char* clientId, const char* refreshToken, const char* prefsNamespace)
      : auth(clientId, refreshToken, prefsNamespace), client(auth) {}

  SpotifyAuth auth;
  SpotifyClient client;
};

// Top-level application: wires the modules together and owns the display state
// machine: cover art while music plays, a rotating set of info screens (clock,
// weather) while idle, and a blank panel after a long idle spell.
//
// Construction only stores configuration; all hardware is touched in setup().
class AlbumArtApp {
 public:
  explicit AlbumArtApp(const Credentials& credentials);

  void setup();
  void loop();

 private:
  void connectNetwork();
  void updateBrightness();
  void pollIfDue();
  void poll();
  void updateIdleScreen();
  void showCover(const String& url);
  void enterIdle();
  void refreshOneDataSource(uint32_t nowMs);
  NowPlaying fetchNowPlaying(SpotifyAccount& account);

  static constexpr size_t kMaxAccounts = 2;

  Display display_;
  WifiManager wifi_;
  SpotifyAccount account1_;
  SpotifyAccount account2_;
  SpotifyAccount* accounts_[kMaxAccounts];
  size_t accountCount_;
  size_t activeAccount_ = 0;  // whose cover is (or was last) on screen
  TimeService time_;

  // Idle-time information screens and the data behind them.
  WeatherService weather_;
  PeriodicSource* dataSources_[1];
  ClockScreen clockScreen_;
  WeatherScreen weatherScreen_;
  Screen* screens_[2];
  ScreenCarousel carousel_;

  Frame current_;       // cover currently on screen
  Frame incoming_;      // cover being decoded
  const Frame blank_{};  // all black

  String currentImageUrl_;
  bool haveCover_ = false;
  bool idleMode_ = true;
  bool screenBlank_ = false;

  uint32_t idleSinceMs_ = 0;
  uint32_t lastPlayingMs_ = 0;
  uint32_t lastPollMs_ = 0;
  uint32_t pollDelayMs_ = 0;  // 0 so the first poll happens immediately
  uint32_t lastIdleDrawMs_ = 0;
  uint32_t lastBrightnessMs_ = 0;
};
