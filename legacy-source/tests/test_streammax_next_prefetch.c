#include "../src/streammax_next_prefetch.h"
#include <assert.h>
int main(void) {
  // Legacy automatic playback may still pre-resolve video.
  assert(streammax_next_video_preload(1, 0));
  assert(!streammax_next_video_preload(0, 0));

  // Manual means never resolve/play a different video invisibly.
  assert(!streammax_next_video_preload(1, 1));
  assert(!streammax_next_video_preload(0, 1));

  // Explicit metadata toggle remains intact, for both modes.
  assert(streammax_next_metadata_prefetch(1, 0, 0, 0));
  assert(streammax_next_metadata_prefetch(1, 0, 1, 0));

  // Video-preload setting plus remembered manual choice allows LIST prefetch.
  assert(streammax_next_metadata_prefetch(0, 1, 1, 1));
  // First selection must NOT trigger speculative addon list or video.
  assert(!streammax_next_metadata_prefetch(0, 1, 1, 0));
  // Both toggles off means no speculation.
  assert(!streammax_next_metadata_prefetch(0, 0, 1, 1));
  assert(!streammax_next_metadata_prefetch(0, 0, 0, 0));
  return 0;
}
