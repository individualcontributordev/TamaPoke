#pragma once

#include <stdint.h>

// Safe comparisons for millis() deadlines. Thanks to the signed difference
// they also work when the 32-bit counter wraps after about 49 days.
inline bool deadlineActive(uint32_t now, uint32_t deadline) {
  return deadline != 0 && (int32_t)(now - deadline) < 0;
}

inline bool deadlineReached(uint32_t now, uint32_t deadline) {
  return deadline != 0 && (int32_t)(now - deadline) >= 0;
}

inline uint32_t deadlineRemaining(uint32_t now, uint32_t deadline) {
  return deadlineActive(now, deadline) ? deadline - now : 0;
}
