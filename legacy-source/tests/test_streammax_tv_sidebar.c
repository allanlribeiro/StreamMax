#include "../src/streammax_tv_sidebar.h"
#include <assert.h>
int main(void) {
  assert(streammax_tv_clamp(-1, 12) == 0);
  assert(streammax_tv_clamp(12, 12) == 11);
  assert(streammax_tv_clamp(3, 12) == 3);
  assert(streammax_tv_clamp(4, 0) == 0);
  assert(streammax_tv_first_visible(0, 100, 10) == 0);
  assert(streammax_tv_first_visible(50, 100, 10) == 45);
  assert(streammax_tv_first_visible(99, 100, 10) == 90);
  assert(streammax_tv_first_visible(8, 8, 10) == 0);
  assert(streammax_tv_first_visible(8, 0, 10) == 0);
  assert(streammax_tv_first_visible(8, 10, 0) == 0);
  assert(STREAMMAX_TV_FOCUS_CATEGORIES != STREAMMAX_TV_FOCUS_CHANNELS);
  return 0;
}
