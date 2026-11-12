// Spotify album-art LED display.
//
// Shows the cover of whatever is playing on Spotify on a 64x64 HUB75 panel,
// crossfading between tracks, with a clock when nothing is playing.
// See README.md for hardware, setup and architecture.

#include <Arduino.h>

#include "AlbumArtApp.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "Missing secrets.h: copy include/secrets.h.example to include/secrets.h and fill it in."
#endif

namespace {

const Credentials kCredentials = {WIFI_SSID, WIFI_PASS, SPOTIFY_CLIENT_ID,
                                  SPOTIFY_REFRESH_TOKEN};

AlbumArtApp app(kCredentials);

}  // namespace

void setup() {
  app.setup();
}

void loop() {
  app.loop();
}
