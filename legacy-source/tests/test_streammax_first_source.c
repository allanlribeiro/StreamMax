#include "../src/streammax_first_source.h"
#include <assert.h>

int main(void) {
  // A new movie/series episode must show sources before touching any URL.
  assert(streammax_should_ask_first_source(1, 4, -1));
  assert(streammax_should_ask_first_source(1, 1, -1));

  // A manually remembered matching source (even index 0) continues directly.
  assert(!streammax_should_ask_first_source(1, 4, 0));
  assert(!streammax_should_ask_first_source(1, 4, 3));

  // When the source disappears, show the list and let the viewer choose again.
  assert(streammax_should_ask_first_source(1, 3, -1));

  // No list yet: wait for add-ons, never show an empty selection dialog.
  assert(!streammax_should_ask_first_source(1, 0, -1));

  // StreamMax always asks before the FIRST source, regardless of a
  // legacy automatic setting. A previous manual choice may resume.
  assert(streammax_should_ask_first_source(0, 4, -1));
  assert(!streammax_should_ask_first_source(0, 4, 0));

  // StreamMax: episode card prompts only before the FIRST manual choice.
  // On subsequent episodes, the playback router checks the source against
  // the actual episode list; a missing match reopens the chooser there.
  assert(streammax_should_ask_episode_source(1, 0));
  assert(!streammax_should_ask_episode_source(1, 1));
  // The legacy toggle must not bypass the mandatory initial choice.
  assert(streammax_should_ask_episode_source(0, 0));
  assert(!streammax_should_ask_episode_source(0, 1));
  return 0;
}
