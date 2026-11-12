#include "SpotifyClient.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "Log.h"
#include "Tls.h"
#include "config.h"

namespace {

// Spotify offers 640, 300 and 64 pixel covers; the panel only needs the smallest.
String pickSmallestImageUrl(JsonArray images) {
  String best;
  int bestWidth = INT32_MAX;
  for (JsonObject image : images) {
    const char* url = image["url"] | "";
    const int width = image["width"] | 0;
    if (*url != '\0' && width > 0 && width < bestWidth) {
      bestWidth = width;
      best = url;
    }
  }
  return best;
}

// Parses the now-playing payload, keeping only the fields we use (the full
// response is several KB of metadata).
bool parsePayload(const String& payload, NowPlaying& out) {
  JsonDocument filter;
  filter["is_playing"] = true;
  filter["item"]["album"]["images"][0]["url"] = true;
  filter["item"]["album"]["images"][0]["width"] = true;
  filter["item"]["images"][0]["url"] = true;  // podcast episodes
  filter["item"]["images"][0]["width"] = true;

  JsonDocument doc;
  const DeserializationError err =
      deserializeJson(doc, payload, DeserializationOption::Filter(filter));
  if (err) {
    LOG_ERROR("spotify", "JSON error: %s", err.c_str());
    return false;
  }

  JsonArray images = doc["item"]["album"]["images"].as<JsonArray>();  // tracks
  if (images.isNull()) {
    images = doc["item"]["images"].as<JsonArray>();  // episodes
  }
  out.imageUrl = pickSmallestImageUrl(images);
  const bool playing = doc["is_playing"] | false;
  out.status = playing ? ledcore::NowStatus::Playing : ledcore::NowStatus::Paused;
  return true;
}

}  // namespace

NowPlaying SpotifyClient::fetchNowPlaying() {
  NowPlaying result;

  WiFiClientSecure client;
  tls::configure(client);
  HTTPClient http;
  http.setTimeout(config::network::kApiRequestTimeoutMs);
  http.setReuse(false);
  if (!http.begin(client, config::spotify::kNowPlayingUrl)) {
    return result;
  }
  http.addHeader("Authorization", "Bearer " + auth_.accessToken());

  const int code = http.GET();
  if (code == 204) {  // nothing is playing
    http.end();
    result.status = ledcore::NowStatus::Nothing;
    return result;
  }
  if (code == 401) {  // access token expired early; refresh on the next poll
    http.end();
    auth_.invalidate();
    return result;
  }
  if (code != 200) {
    LOG_WARN("spotify", "now-playing HTTP %d", code);
    http.end();
    return result;
  }

  const String payload = http.getString();
  http.end();
  if (!parsePayload(payload, result)) {
    result.status = ledcore::NowStatus::Error;
  }
  return result;
}
