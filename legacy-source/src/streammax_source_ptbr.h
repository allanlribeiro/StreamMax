// StreamMax: fallback limitado para a folha de fontes com audio nao declarado.
// Apenas evidencias do proprio nome/descricao, nunca legendas nem URL.
// O filtro explicito de idioma continua sendo a autoridade em streams.c.
#ifndef STREAMMAX_SOURCE_PTBR_H
#define STREAMMAX_SOURCE_PTBR_H
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>

// Exige limites de palavra: "dublado" sim; "redublado123" nao.
// Ponto, sublinhado, hifen e espaco sao separadores de releases.
static inline int streammax_pt_token(const char *texto, const char *marca) {
  size_t n;
  if (!texto || !marca || !(n = strlen(marca))) return 0;
  for (const char *p = texto; *p; p++) {
    if (p != texto && isalnum((unsigned char)p[-1])) continue;
    if (!strncasecmp(p, marca, n) && !isalnum((unsigned char)p[n])) return 1;
  }
  return 0;
}

static inline int streammax_pt_dica_explicitamente_brasileira(const char *s) {
  return streammax_pt_token(s, "pt-br") || streammax_pt_token(s, "pt_br") ||
         streammax_pt_token(s, "ptbr") || streammax_pt_token(s, "pob") ||
         streammax_pt_token(s, "brazilian portuguese") ||
         streammax_pt_token(s, "portuguese brazilian") ||
         streammax_pt_token(s, "portuguese (brazil)") ||
         streammax_pt_token(s, "portuguese (brasil)") ||
         streammax_pt_token(s, "português brasileiro") ||
         streammax_pt_token(s, "portugues brasileiro") ||
         streammax_pt_token(s, "português (brasil)") ||
         streammax_pt_token(s, "portuguese (br)") ||
         (s && strstr(s, "🇧🇷") != NULL);
}

static inline int streammax_pt_dica_dublagem(const char *s) {
  return streammax_pt_token(s, "dublado") ||
         streammax_pt_token(s, "dublada") ||
         streammax_pt_token(s, "dublagem");
}

// Uma descricao pode relatar varias trilhas: "Subs: PT-BR" nao e audio.
// Dividir por linha impede que uma legenda brasileira vire dublagem.
static inline int streammax_pt_descricao_audio(const char *descricao) {
  const char *p = descricao;
  if (!p) return 0;
  while (*p) {
    char linha[1024];
    size_t n = 0;
    while (*p && *p != '\n') {
      if (n + 1 < sizeof linha) linha[n++] = *p;
      p++;
    }
    if (*p) p++;
    linha[n] = 0;
    const char *v = linha;
    while (*v == ' ' || *v == '\t') v++;
    if (!strncasecmp(v, "subs:", 5) || !strncasecmp(v, "subtitle:", 9) ||
        !strncasecmp(v, "subtitles:", 10) ||
        !strncasecmp(v, "legendas:", 9) || !strncasecmp(v, "legenda:", 8))
      continue;
    if (streammax_pt_dica_dublagem(v)) return 1;
    if ((!strncasecmp(v, "audio:", 6) ||
         !strncasecmp(v, "audio languages:", 16) ||
         !strncasecmp(v, "language:", 9) ||
         !strncasecmp(v, "idioma:", 7)) &&
        (streammax_pt_dica_explicitamente_brasileira(v) ||
         streammax_pt_token(v, "portuguese") ||
         streammax_pt_token(v, "português") ||
         streammax_pt_token(v, "portugues") ||
         streammax_pt_token(v, "pt")))
      return 1;
  }
  return 0;
}

// Tag de idioma seguida so por SUBS nao comprova o audio brasileiro.
static inline int streammax_pt_marca_release(const char *s) {
  if (streammax_pt_dica_dublagem(s)) return 1;
  if (streammax_pt_token(s, "subs") || streammax_pt_token(s, "subtitle") ||
      streammax_pt_token(s, "subtitles") || streammax_pt_token(s, "legendado") ||
      streammax_pt_token(s, "legendas"))
    return 0;
  return streammax_pt_dica_explicitamente_brasileira(s);
}

// Somente quando NAO ha nenhuma lista formal de audio declarada pelo addon.
// Nunca transforma uma legenda em audio nem sobrescreve audio ES/EN/PT-PT.
static inline int streammax_pt_audio_sem_declaracao(
    const char *arquivo, const char *rotulo, const char *descricao) {
  return streammax_pt_marca_release(arquivo) ||
         streammax_pt_marca_release(rotulo) ||
         streammax_pt_descricao_audio(descricao);
}
#endif
