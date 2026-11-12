#pragma once

#include <Arduino.h>
#include <ledcore/PlaybackPolicy.h>

#include "SpotifyAuth.h"

struct NowPlaying {
  ledcore::NowStatus status = ledcore::NowStatus::Error;
  String imageUrl;  // smallest available cover (empty if none)
};

// Reads the user's current playback state from the Spotify Web API.
class SpotifyClient {
 public:
  explicit SpotifyClient(SpotifyAuth& auth) : auth_(auth) {}

  NowPlaying fetchNowPlaying();

 private:
  SpotifyAuth& auth_;
};
