// Time-of-day helpers.
#pragma once

namespace ledcore {

// True when `hour` (0-23) falls in the night window [nightStart, nightEnd).
// The window may wrap past midnight, e.g. 23 -> 7.
inline bool isNightHour(int hour, int nightStart, int nightEnd) {
  if (nightStart > nightEnd) {
    return hour >= nightStart || hour < nightEnd;
  }
  return hour >= nightStart && hour < nightEnd;
}

}  // namespace ledcore
