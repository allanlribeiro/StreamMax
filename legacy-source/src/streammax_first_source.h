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
  (void)manualEnabled;
  return availableStreams > 0 && rememberedIndex < 0;
}

 // A series episode short-click is a playback request, not an explicit
 // "change source" action. After one manual choice, route it through the
 // normal VOD resolver: it validates the remembered source against the new
 // episode's addon list and reopens the picker if the match disappeared.
 // Long-press "Fontes deste episodio" still opens the source picker.
static inline int streammax_should_ask_episode_source(int manualEnabled,
                                                      int hasManualPreference) {
  (void)manualEnabled;
  return !hasManualPreference;
}

// A busca pode terminar com o seletor ainda aberto. Nenhum timeout, refresh
// de addon ou resposta tardia autoriza reproduzir sem OK na fonte.
// A TV ao vivo usa outro fluxo e continua escolhendo pela playlist.
static inline int streammax_auto_may_play(int isLiveTV,
                                         int pickerVisible,
                                         int awaitingManualChoice) {
  return isLiveTV || (!pickerVisible && !awaitingManualChoice);
}

#endif
