#pragma once

#include <Arduino.h>
#include <Preferences.h>

// Keeps a valid Spotify access token available using the OAuth refresh-token flow.
//
// Spotify may rotate the refresh token on any refresh. The newest one is stored
// in flash (NVS) so the display survives reboots without re-running the login.
class SpotifyAuth {
 public:
  // prefsNamespace keeps each account's rotated token separate in flash
  // (NVS namespaces are at most 15 characters).
  SpotifyAuth(const char* clientId, const char* seedRefreshToken, const char* prefsNamespace);

  // Opens flash storage and picks the refresh token to use. Call once in setup().
  void begin();

  // Returns true if a usable access token is available, refreshing if needed.
  bool ensureAccessToken();

  // Forces a refresh on the next ensureAccessToken() (call after an HTTP 401).
  void invalidate();

  const String& accessToken() const { return accessToken_; }

 private:
  bool refresh();
  void loadRefreshToken();

  const char* clientId_;
  const char* seedRefreshToken_;
  const char* prefsNamespace_;
  Preferences prefs_;
  String refreshToken_;
  String accessToken_;
  uint32_t refreshAtMs_ = 0;
  bool haveToken_ = false;
};
