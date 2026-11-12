#include "ScreenCarousel.h"

#include <ledcore/Rotation.h>

void ScreenCarousel::tick(Display& display, uint32_t nowMs) {
  if (count_ == 0 || count_ > kMaxScreens) return;

  if (!started_) {
    started_ = true;
    index_ = 0;
    shownAtMs_ = nowMs;
  } else {
    const bool timeUp = nowMs - shownAtMs_ >= screens_[index_]->durationMs();
    if (timeUp || !screens_[index_]->isAvailable(nowMs)) {
      bool available[kMaxScreens] = {};
      for (size_t i = 0; i < count_; i++) {
        available[i] = screens_[i]->isAvailable(nowMs);
      }
      index_ = ledcore::nextAvailable(available, count_, index_);
      shownAtMs_ = nowMs;
    }
  }
  screens_[index_]->draw(display, nowMs);
}
