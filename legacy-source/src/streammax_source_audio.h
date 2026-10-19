// StreamMax: sinais explicitos de Dual Audio no proprio stream recebido.
// Nao inferir idiomas, bandeiras ou dublagem a partir do selo.
#ifndef STREAMMAX_SOURCE_AUDIO_H
#define STREAMMAX_SOURCE_AUDIO_H
#include "streammax_source_ptbr.h"
#include <stdio.h>

static inline int streammax_audio_dual_texto(const char *s) {
  // Releases Torrentio/TorBox: separadores como ponto e hifen sao comuns.
  static const char *const tags[] = {
    "dual audio", "dual-audio", "dual.audio", "dual_audio", "dualaudio",
    "audio dual", "audio-dual", "audio.dual", "audio_dual",
    "áudio dual", "áudio-dual", "áudio.dual", "áudio_dual",
    "Áudio dual", "Áudio-dual", "Áudio.dual", "Áudio_dual",
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

// Retorna APENAS os idiomas explicitamente escritos junto da marca de audio
// no proprio nome/arquivo/descricao da fonte. Nao deduz ingles de "Dual"
// nem portugues de "TB", qualidade, URL, debrid ou legendas.
#define SM_DUAL_BR 1u
#define SM_DUAL_EN 2u
#define SM_DUAL_PT 4u    // portugues generico; nao transformar em PT-BR no filtro
#define SM_DUAL_PTPT 8u  // portugues de Portugal, mostrado com bandeira BR
static inline unsigned streammax_audio_dual_idiomas_linha(const char *txt) {
  const char *p = txt;
  char linha[1100];
  size_t n = 0;
  if (!p || !*p) return 0;
  // Secoes de subs/legendas NUNCA contam como faixas de audio, mesmo que
  // tragam 🇧🇷, PT-BR ou EN no meio da descricao do Torrentio.
  for (; *p && *p != '\n' && *p != '\r' && n + 1 < sizeof linha; p++) {
    if (streammax_pt_token(p, "subs") || streammax_pt_token(p, "subtitles") ||
        streammax_pt_token(p, "legendas") || streammax_pt_token(p, "legendado") ||
        streammax_pt_token(p, "subtitle:")) break;
    linha[n++] = *p;
  }
  linha[n] = 0;
  if (!streammax_audio_dual_texto(linha) &&
      !streammax_pt_token(linha, "audio:") &&
      !streammax_pt_token(linha, "áudio:") &&
      !streammax_pt_token(linha, "audio languages:") &&
      !streammax_pt_token(linha, "audio languages") &&
      !streammax_pt_token(linha, "dual")) return 0;
  unsigned bits = 0;
  if (streammax_pt_dica_explicitamente_brasileira(linha)) bits |= SM_DUAL_BR;
  else if (strstr(linha, "🇵🇹") ||
           streammax_pt_token(linha, "pt-pt") ||
           streammax_pt_token(linha, "pt_pt") ||
           streammax_pt_token(linha, "portuguese (portugal)") ||
           streammax_pt_token(linha, "portuguese (pt)") ||
           streammax_pt_token(linha, "português (portugal)") ||
           streammax_pt_token(linha, "português (pt)"))
    bits |= SM_DUAL_PTPT;
  else if (streammax_pt_token(linha, "portuguese") ||
           streammax_pt_token(linha, "portugues") ||
           streammax_pt_token(linha, "português") ||
           streammax_pt_token(linha, "por") ||
           streammax_pt_token(linha, "pt"))
    bits |= SM_DUAL_PT;
  if (streammax_pt_token(linha, "english") ||
      streammax_pt_token(linha, "inglês") ||
      streammax_pt_token(linha, "ingles") ||
      streammax_pt_token(linha, "eng") ||
      streammax_pt_token(linha, "en") ||
      strstr(linha, "🇬🇧") || strstr(linha, "🇺🇸")) bits |= SM_DUAL_EN;
  return bits;
}
static inline unsigned streammax_audio_dual_idiomas_texto(const char *txt) {
  unsigned bits = 0;
  if (!txt) return 0;
  while (*txt) {
    char linha[1100];
    size_t n = 0;
    while (*txt && *txt != '\n' && *txt != '\r') {
      if (n + 1 < sizeof linha) linha[n++] = *txt;
      txt++;
    }
    linha[n] = 0;
    // Alguns addons dividem o selo Dual Audio e as bandeiras em linhas
    // consecutivas. A linha seguinte so vale quando e explicitamente de
    // audio, e nunca quando se trata de legendas.
    if (streammax_audio_dual_texto(linha)) {
      bits |= streammax_audio_dual_idiomas_linha(linha);
      const char *next = txt;
      if (*next) next++;
      if (*next) {
        char seguinte[1100];
        size_t j = 0;
        while (next[j] && next[j] != '\\n' && next[j] != '\\r' &&
               j + 1 < sizeof seguinte) {
          seguinte[j] = next[j];
          j++;
        }
        seguinte[j] = 0;
        if ((strstr(seguinte, "🇵🇹") || strstr(seguinte, "🇧🇷") ||
             strstr(seguinte, "🇬🇧") || strstr(seguinte, "🇺🇸")) &&
            !streammax_pt_token(seguinte, "subs") &&
            !streammax_pt_token(seguinte, "subtitle") &&
            !streammax_pt_token(seguinte, "legendas")) {
          char junto[2200];
          snprintf(junto, sizeof junto, "Dual Audio %s", seguinte);
          bits |= streammax_audio_dual_idiomas_linha(junto);
        }
      }
    } else {
      bits |= streammax_audio_dual_idiomas_linha(linha);
    }
    if (*txt) txt++;
  }
  return bits;
}
static inline unsigned streammax_audio_dual_idiomas_fonte(const char *arquivo,
                                                           const char *nome,
                                                           const char *titulo,
                                                           const char *descricao) {
  // Detectar marcas apenas se a fonte DECLAROU dual audio.
  if (!streammax_audio_dual_fonte(arquivo, nome, titulo, descricao)) return 0;
  return streammax_audio_dual_idiomas_texto(arquivo) |
         streammax_audio_dual_idiomas_texto(nome) |
         streammax_audio_dual_idiomas_texto(titulo) |
         streammax_audio_dual_idiomas_texto(descricao);
}
#endif
