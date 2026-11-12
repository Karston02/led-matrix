#include "PeriodicSource.h"

#include "Log.h"

PeriodicSource::PeriodicSource(const char* name, uint32_t intervalMs, uint32_t retryMs)
    : name_(name), intervalMs_(intervalMs), retryMs_(retryMs) {}

bool PeriodicSource::isDue(uint32_t nowMs) const {
  if (!attempted_) return true;
  const uint32_t wait = lastAttemptOk_ ? intervalMs_ : retryMs_;
  return nowMs - lastAttemptMs_ >= wait;
}

void PeriodicSource::refresh(uint32_t nowMs) {
  attempted_ = true;
  lastAttemptMs_ = nowMs;
  lastAttemptOk_ = fetch();
  if (lastAttemptOk_) {
    everSucceeded_ = true;
    lastSuccessMs_ = nowMs;
  } else {
    LOG_WARN("data", "%s refresh failed, will retry", name_);
  }
}

bool PeriodicSource::hasRecentData(uint32_t nowMs) const {
  return everSucceeded_ && nowMs - lastSuccessMs_ <= 3 * intervalMs_;
}
