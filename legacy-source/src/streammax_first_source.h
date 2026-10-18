#ifndef STREAMMAX_FIRST_SOURCE_H
#define STREAMMAX_FIRST_SOURCE_H

// StreamMax VOD entry policy (Android TV).
// The first explicit source choice belongs to the viewer. Once a manual
// preference for this title/profile matches a stream in the current list,
// the next episode can continue without opening the picker again.
// Live TV is routed separately in app.c and must not enter this policy.
static inline int streammax_should_ask_first_source(int manualEnabled,
                                                     int availableStreams,
                                                     int rememberedIndex) {
  return manualEnabled && availableStreams > 0 && rememberedIndex < 0;
}

#endif
