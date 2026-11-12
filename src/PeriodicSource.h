#pragma once

#include <Arduino.h>

// A network data source that refreshes on a schedule.
//
// Subclasses implement fetch(); this class decides when to call it, retries
// failures sooner than successes, and reports whether the data is still fresh
// enough to show.
class PeriodicSource {
 public:
  PeriodicSource(const char* name, uint32_t intervalMs, uint32_t retryMs);
  virtual ~PeriodicSource() = default;
  PeriodicSource(const PeriodicSource&) = delete;
  PeriodicSource& operator=(const PeriodicSource&) = delete;

  bool isDue(uint32_t nowMs) const;
  void refresh(uint32_t nowMs);

  // True if the last good fetch is younger than three refresh intervals.
  bool hasRecentData(uint32_t nowMs) const;

 protected:
  // Blocking network fetch. Return true on success.
  virtual bool fetch() = 0;

 private:
  const char* name_;
  uint32_t intervalMs_;
  uint32_t retryMs_;
  bool attempted_ = false;
  bool lastAttemptOk_ = false;
  bool everSucceeded_ = false;
  uint32_t lastAttemptMs_ = 0;
  uint32_t lastSuccessMs_ = 0;
};
