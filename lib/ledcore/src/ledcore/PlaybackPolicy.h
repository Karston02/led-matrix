// Decides what the display should do after each Spotify poll.
//
// Kept free of hardware and I/O so every branch can be unit-tested on a desktop.
#pragma once

#include <cstddef>
#include <cstdint>

namespace ledcore {

enum class NowStatus { Error, Nothing, Playing, Paused };

// Picks which account's playback to show when several accounts are polled.
//
// The account on screen keeps the panel while it is playing, so two people
// listening at once don't make it flip back and forth. Otherwise the first
// account that is playing takes over. If none is playing, the account on screen
// is kept, so its pause grace period and idle fade behave as with one account.
inline size_t chooseAccount(const NowStatus* statuses, size_t count, size_t current) {
  if (current < count && statuses[current] == NowStatus::Playing) {
    return current;
  }
  for (size_t i = 0; i < count; ++i) {
    if (statuses[i] == NowStatus::Playing) {
      return i;
    }
  }
  return current < count ? current : 0;
}

struct PollTiming {
  uint32_t playingMs;  // poll interval while music is (or was just) playing
  uint32_t idleMs;     // poll interval while showing the clock
  uint32_t errorMs;    // back-off after a failed request
  uint32_t graceMs;    // keep showing the cover this long after pause/stop
};

struct PollContext {
  NowStatus status;
  bool idleMode;         // display is currently showing the clock / blank
  bool hasImage;         // the response carried a cover URL
  bool imageChanged;     // that URL differs from the one on screen
  uint32_t msSincePlaying;
};

struct PollDecision {
  uint32_t nextPollMs;
  bool showArt;    // fetch and fade to the new cover
  bool enterIdle;  // fade out and switch to the clock
};

inline PollDecision decidePoll(const PollContext& ctx, const PollTiming& timing) {
  PollDecision decision{timing.errorMs, false, false};
  const bool graceExpired = !ctx.idleMode && ctx.msSincePlaying > timing.graceMs;

  switch (ctx.status) {
    case NowStatus::Playing:
      decision.nextPollMs = timing.playingMs;
      decision.showArt = ctx.hasImage && ctx.imageChanged;
      break;
    case NowStatus::Paused:
      // A paused track keeps its cover on screen until the grace period ends.
      decision.nextPollMs = timing.playingMs;
      decision.showArt = !ctx.idleMode && ctx.hasImage && ctx.imageChanged;
      decision.enterIdle = graceExpired;
      break;
    case NowStatus::Nothing:
      decision.nextPollMs = ctx.idleMode ? timing.idleMs : timing.playingMs;
      decision.enterIdle = graceExpired;
      break;
    case NowStatus::Error:
      break;
  }
  return decision;
}

}  // namespace ledcore
