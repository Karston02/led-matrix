// Selection helper for the idle-screen carousel.
#pragma once

#include <cstddef>
#include <cstdint>

namespace ledcore {

// Index of the next available item after `current`, wrapping around.
// Returns `current` if nothing else is available.
inline size_t nextAvailable(const bool* available, size_t count, size_t current) {
  for (size_t step = 1; step <= count; step++) {
    const size_t index = (current + step) % count;
    if (available[index]) return index;
  }
  return current;
}

}  // namespace ledcore
