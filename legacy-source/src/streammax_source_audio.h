// StreamMax: sinais explicitos de Dual Audio no proprio stream recebido.
// Nao inferir idiomas, bandeiras ou dublagem a partir do selo.
#ifndef STREAMMAX_SOURCE_AUDIO_H
#define STREAMMAX_SOURCE_AUDIO_H
#include "streammax_source_ptbr.h"

static inline int streammax_audio_dual_texto(const char *s) {
  // Releases Torrentio/TorBox: separadores como ponto e hifen sao comuns.
  static const char *const tags[] = {
    "dual audio", "dual-audio", "dual.audio", "dual_audio", "dualaudio",
    "audio dual", "audio-dual", "audio.dual", "audio_dual",
    "áudio dual", "áudio-dual", "áudio.dual", "áudio_dual",
  };
  if (!s) return 0;
  for (unsigned i = 0; i < sizeof tags / sizeof *tags; i++)
    if (streammax_pt_token(s, tags[i])) return 1;
  return 0;
}
static inline int streammax_audio_dual_fonte(const char *arquivo,
                                             const char *nome,
                                             const char *titulo,
                                             const char *descricao) {
  // "DUAL" isolado e convencao de release apenas no nome do ARQUIVO.
  // No nome do titulo pode ser parte do nome de um filme.
  return streammax_audio_dual_texto(arquivo) ||
         (arquivo && streammax_pt_token(arquivo, "dual")) ||
         streammax_audio_dual_texto(nome) ||
         streammax_audio_dual_texto(titulo) ||
         streammax_audio_dual_texto(descricao);
}
#endif
