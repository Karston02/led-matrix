#pragma once

#include <time.h>

// Wall-clock time via NTP in the configured time zone.
class TimeService {
 public:
  // Starts background NTP sync. Safe to call before Wi-Fi is up.
  void begin();

  // Fills `out` and returns true once the clock has synced.
  bool localTime(tm& out) const;

};
