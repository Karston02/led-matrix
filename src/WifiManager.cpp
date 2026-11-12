#include "WifiManager.h"

#include <WiFi.h>

#include "Log.h"
#include "config.h"

WifiManager::WifiManager(const char* ssid, const char* password)
    : ssid_(ssid), password_(password) {}

bool WifiManager::connect(uint32_t timeoutMs) {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);  // power-save adds latency and occasional dropouts
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid_, password_);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
    delay(250);
  }
  return WiFi.status() == WL_CONNECTED;
}

void WifiManager::maintain() {
  if (isConnected()) {
    lostSinceMs_ = 0;
    return;
  }
  const uint32_t now = millis();
  if (lostSinceMs_ == 0) {
    lostSinceMs_ = now;
  }
  if (now - lostSinceMs_ > config::network::kWifiReconnectAfterMs) {
    LOG_WARN("wifi", "link lost, reconnecting");
    WiFi.disconnect();
    WiFi.begin(ssid_, password_);
    lostSinceMs_ = now;
  }
}

bool WifiManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

String WifiManager::localIp() const {
  return WiFi.localIP().toString();
}
