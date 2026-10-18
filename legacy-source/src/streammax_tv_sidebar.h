#ifndef STREAMMAX_TV_SIDEBAR_H
#define STREAMMAX_TV_SIDEBAR_H

// Small, deterministic D-pad / viewport primitives used by the TCL guide.
// Neither the navigation nor the viewport allocates or touches video streams.
enum { STREAMMAX_TV_FOCUS_CATEGORIES = 0, STREAMMAX_TV_FOCUS_CHANNELS = 1 };

static inline int streammax_tv_clamp(int value, int total) {
  if (total <= 0) return 0;
  if (value < 0) return 0;
  return value >= total ? total - 1 : value;
}

static inline int streammax_tv_first_visible(int focused, int total, int capacity) {
  if (total <= 0 || capacity <= 0) return 0;
  if (capacity >= total) return 0;
  focused = streammax_tv_clamp(focused, total);
  int first = focused - capacity / 2;
  if (first < 0) first = 0;
  if (first + capacity > total) first = total - capacity;
  return first;
}

#endif
