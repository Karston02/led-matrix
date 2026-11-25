#include "AlbumArtApp.h"

#include <ledcore/Schedule.h>

#include "CoverArt.h"
#include "Log.h"
#include "config.h"

namespace {

constexpr uint32_t kSerialBaud = 115200;
constexpr uint32_t kBootDelayMs = 1500;  // lets the USB serial monitor attach

const ledcore::PollTiming kPollTiming = {
    config::timing::kPollPlayingMs,
    config::timing::kPollIdleMs,
    config::timing::kPollErrorMs,
    config::timing::kIdleGraceMs,
};

[[noreturn]] void halt() {
  while (true) {
    delay(1000);
  }
}

}  // namespace

AlbumArtApp::AlbumArtApp(const Credentials& credentials)
    : wifi_(credentials.wifiSsid, credentials.wifiPassword),
      // Account 1 keeps the original "spotify" namespace so an existing stored token survives.
      account1_(credentials.spotifyClientId, credentials.spotifyRefreshToken, "spotify"),
      account2_(credentials.spotifyClientId, credentials.spotifyRefreshToken2, "spotify2"),
      accounts_{&account1_, &account2_},
      accountCount_(*credentials.spotifyRefreshToken2 != '\0' ? 2 : 1),
      dataSources_{&weather_},
      clockScreen_(time_),
      weatherScreen_(weather_),
      screens_{&clockScreen_, &weatherScreen_},
      carousel_(screens_, 2) {}

void AlbumArtApp::setup() {
  Serial.begin(kSerialBaud);
  delay(kBootDelayMs);
  LOG_INFO("app", "LED album art starting");

  if (!display_.begin()) {
    LOG_ERROR("app", "panel init failed");
    halt();
  }
  display_.setBrightness(config::display::kBrightnessDay);

  display_.showMessage("WiFi...");
  for (size_t i = 0; i < accountCount_; ++i) {
    accounts_[i]->auth.begin();
  }
  LOG_INFO("app", "following %u Spotify account(s)", static_cast<unsigned>(accountCount_));
  time_.begin();
  connectNetwork();
  idleSinceMs_ = millis();
}

void AlbumArtApp::loop() {
  wifi_.maintain();
  updateBrightness();
  pollIfDue();
  updateIdleScreen();
  delay(20);
}

void AlbumArtApp::connectNetwork() {
  if (wifi_.connect(config::network::kWifiConnectTimeoutMs)) {
    LOG_INFO("wifi", "connected, IP %s", wifi_.localIp().c_str());
    display_.showMessage("Spotify...");
  } else {
    LOG_WARN("wifi", "connection failed, will keep retrying");
    display_.showMessage("WiFi", "failed");
    delay(2000);
  }
}

void AlbumArtApp::updateBrightness() {
  const uint32_t now = millis();
  if (now - lastBrightnessMs_ <= config::display::kBrightnessCheckMs) {
    return;
  }
  lastBrightnessMs_ = now;

  uint8_t level = config::display::kBrightnessDay;
  tm local;
  if (time_.localTime(local) &&
      ledcore::isNightHour(local.tm_hour, config::display::kNightStartHour,
                           config::display::kNightEndHour)) {
    level = config::display::kBrightnessNight;
  }
  display_.setBrightness(level);
}

void AlbumArtApp::pollIfDue() {
  if (millis() - lastPollMs_ < pollDelayMs_) {
    return;
  }
  lastPollMs_ = millis();
  poll();
}

void AlbumArtApp::poll() {
  if (!wifi_.isConnected()) {
    pollDelayMs_ = config::timing::kPollWifiDownMs;
    return;
  }

  NowPlaying results[kMaxAccounts];
  ledcore::NowStatus statuses[kMaxAccounts];
  for (size_t i = 0; i < accountCount_; ++i) {
    results[i] = fetchNowPlaying(*accounts_[i]);
    statuses[i] = results[i].status;
  }
  const size_t chosen = ledcore::chooseAccount(statuses, accountCount_, activeAccount_);
  if (chosen != activeAccount_) {
    LOG_INFO("app", "switching to account %u", static_cast<unsigned>(chosen + 1));
    activeAccount_ = chosen;
  }

  const NowPlaying& now = results[chosen];
  const uint32_t nowMs = millis();
  if (now.status == ledcore::NowStatus::Playing) {
    lastPlayingMs_ = nowMs;
  }

  const ledcore::PollContext context = {
      now.status,
      idleMode_,
      now.imageUrl.length() > 0,
      now.imageUrl != currentImageUrl_,
      nowMs - lastPlayingMs_,
  };
  const ledcore::PollDecision decision = ledcore::decidePoll(context, kPollTiming);

  pollDelayMs_ = decision.nextPollMs;
  if (decision.showArt) {
    showCover(now.imageUrl);
  }
  if (decision.enterIdle) {
    enterIdle();
  }
}

NowPlaying AlbumArtApp::fetchNowPlaying(SpotifyAccount& account) {
  if (!account.auth.ensureAccessToken()) {
    return NowPlaying{};  // status Error: back off, leave the screen alone
  }
  return account.client.fetchNowPlaying();
}

void AlbumArtApp::showCover(const String& url) {
  if (!cover_art::fetch(url, incoming_)) {
    return;
  }
  LOG_INFO("app", "new cover");
  display_.crossfade(haveCover_ ? current_ : blank_, incoming_);
  current_ = incoming_;
  haveCover_ = true;
  idleMode_ = false;
  screenBlank_ = false;
  currentImageUrl_ = url;
}

void AlbumArtApp::enterIdle() {
  LOG_INFO("app", "idle");
  if (haveCover_) {
    display_.crossfade(current_, blank_);
  }
  haveCover_ = false;
  currentImageUrl_ = "";
  idleMode_ = true;
  screenBlank_ = false;
  idleSinceMs_ = millis();
  lastIdleDrawMs_ = 0;  // draw the first idle screen right away
  carousel_.reset();
}

void AlbumArtApp::updateIdleScreen() {
  if (!idleMode_ || screenBlank_) {
    return;
  }
  const uint32_t now = millis();
  if (now - idleSinceMs_ > config::timing::kIdleBlankMs) {
    display_.clear();
    screenBlank_ = true;
    return;
  }
  if (now - lastIdleDrawMs_ < config::carousel::kRedrawMs) {
    return;
  }
  lastIdleDrawMs_ = now;

  refreshOneDataSource(now);
  carousel_.tick(display_, millis());
}

// Network fetches block for a second or two, so at most one runs per redraw.
void AlbumArtApp::refreshOneDataSource(uint32_t nowMs) {
  if (!wifi_.isConnected()) {
    return;
  }
  for (PeriodicSource* source : dataSources_) {
    if (source->isDue(nowMs)) {
      source->refresh(nowMs);
      return;
    }
  }
}
