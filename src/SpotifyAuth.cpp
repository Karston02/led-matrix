#include "SpotifyAuth.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ledcore/UrlEncode.h>

#include <string>

#include "Log.h"
#include "Tls.h"
#include "config.h"

namespace {

constexpr const char* kPrefsSeedKey = "seed";    // the token that came from secrets.h
constexpr const char* kPrefsRotatedKey = "rt";   // newest token Spotify handed back
constexpr int kDefaultExpirySeconds = 3600;
constexpr int kExpiryMarginSeconds = 120;  // refresh a little early
constexpr int kMinRefreshSeconds = 60;

String encode(const String& value) {
  return String(ledcore::urlEncode(std::string(value.c_str())).c_str());
}

}  // namespace

SpotifyAuth::SpotifyAuth(const char* clientId, const char* seedRefreshToken,
                         const char* prefsNamespace)
    : clientId_(clientId), seedRefreshToken_(seedRefreshToken), prefsNamespace_(prefsNamespace) {}

void SpotifyAuth::begin() {
  prefs_.begin(prefsNamespace_, false);
  loadRefreshToken();
}

// If a different token is put in secrets.h later, it wins over the stored one.
void SpotifyAuth::loadRefreshToken() {
  if (prefs_.getString(kPrefsSeedKey, "") == seedRefreshToken_) {
    const String rotated = prefs_.getString(kPrefsRotatedKey, "");
    if (rotated.length() > 0) {
      refreshToken_ = rotated;
      return;
    }
  }
  refreshToken_ = seedRefreshToken_;
  prefs_.putString(kPrefsSeedKey, seedRefreshToken_);
  prefs_.remove(kPrefsRotatedKey);
}

bool SpotifyAuth::ensureAccessToken() {
  const bool expired = static_cast<int32_t>(millis() - refreshAtMs_) >= 0;
  if (haveToken_ && !expired) {
    return true;
  }
  return refresh();
}

void SpotifyAuth::invalidate() {
  haveToken_ = false;
}

bool SpotifyAuth::refresh() {
  WiFiClientSecure client;
  tls::configure(client);
  HTTPClient http;
  http.setTimeout(config::network::kTokenRequestTimeoutMs);
  if (!http.begin(client, config::spotify::kTokenUrl)) {
    return false;
  }
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  const String body = "grant_type=refresh_token&client_id=" + encode(clientId_) +
                      "&refresh_token=" + encode(refreshToken_);
  const int code = http.POST(body);
  const String response = (code > 0) ? http.getString() : String();
  http.end();

  if (code != 200) {
    // Response bodies are not logged: they can echo request details.
    LOG_ERROR("auth", "%s: token refresh failed (HTTP %d)", prefsNamespace_, code);
    return false;
  }

  JsonDocument doc;
  if (deserializeJson(doc, response)) {
    LOG_ERROR("auth", "%s: token response was not valid JSON", prefsNamespace_);
    return false;
  }
  const char* token = doc["access_token"] | "";
  if (*token == '\0') {
    LOG_ERROR("auth", "%s: token response had no access_token", prefsNamespace_);
    return false;
  }

  accessToken_ = token;
  const int expiresIn = doc["expires_in"] | kDefaultExpirySeconds;
  refreshAtMs_ = millis() + static_cast<uint32_t>(max(kMinRefreshSeconds,
                                                      expiresIn - kExpiryMarginSeconds)) * 1000UL;
  haveToken_ = true;

  const char* rotated = doc["refresh_token"] | "";
  if (*rotated != '\0' && refreshToken_ != rotated) {
    refreshToken_ = rotated;
    prefs_.putString(kPrefsRotatedKey, refreshToken_);
    LOG_INFO("auth", "%s: stored rotated refresh token", prefsNamespace_);
  }
  LOG_INFO("auth", "%s: access token refreshed", prefsNamespace_);
  return true;
}
