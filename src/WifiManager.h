#pragma once

#include <Arduino.h>

// Connects to a Wi-Fi network and recovers from dropped connections.
class WifiManager {
 public:
  WifiManager(const char* ssid, const char* password);

  // Blocks until connected or the timeout elapses. Returns true on success.
  bool connect(uint32_t timeoutMs);

  // Call from loop(): re-joins the network if the link has been down too long.
  void maintain();

  bool isConnected() const;
  String localIp() const;

 private:
  const char* ssid_;
  const char* password_;
  uint32_t lostSinceMs_ = 0;
};
