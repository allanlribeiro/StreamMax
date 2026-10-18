#ifndef STREAMMAX_NEXT_PREFETCH_H
#define STREAMMAX_NEXT_PREFETCH_H

// Nuvio-original-like next-episode startup on low-RAM Android TV.
// Preparing a video in manual source mode can silently select another
// provider on failure. Never resolve/preload another VOD source before the
// viewer gets to choose it. Metadata may still be fetched ahead of time.
static inline int streammax_next_video_preload(int videoEnabled, int manualSource) {
  return videoEnabled && !manualSource;
}

// If the user enabled full-video preloading but selected a manual source,
// reuse that intent to prefetch ONLY the addon list (no debrid resolution,
// no video player, no surprise fallback). The foreground player continues
// to match the actual chosen source with the next episode after the click.
static inline int streammax_next_metadata_prefetch(
    int metadataEnabled, int videoEnabled, int manualSource, int rememberedSource) {
  return metadataEnabled || (videoEnabled && manualSource && rememberedSource);
}

#endif
