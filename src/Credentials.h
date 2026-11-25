#pragma once

// Secrets handed to the application at construction time. Only main.cpp reads
// secrets.h; no other module depends on it, which keeps them testable and makes
// it obvious where credentials enter the program.
struct Credentials {
  const char* wifiSsid;
  const char* wifiPassword;
  const char* spotifyClientId;
  const char* spotifyRefreshToken;
  const char* spotifyRefreshToken2;  // optional second account; "" if unused
};
