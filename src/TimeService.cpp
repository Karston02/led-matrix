#include "TimeService.h"

#include <Arduino.h>

#include "config.h"

void TimeService::begin() {
  configTzTime(config::clock::kTimezone, config::clock::kNtpPrimary,
               config::clock::kNtpSecondary);
}

bool TimeService::localTime(tm& out) const {
  return getLocalTime(&out, 0);
}
