// StreamMax: bandeiras apenas para exibicao; NAO participa do filtro de audio.
#ifndef STREAMMAX_FLAGS_H
#define STREAMMAX_FLAGS_H
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>

static inline int streammax_flag_idioma_legivel(const char *s) {
  // "/" e pontuacao isolada nao sao linguas: nao exibir separadores vazios.
  if (!s) return 0;
  for (; *s; s++) {
    unsigned char c = (unsigned char)*s;
    if (isalnum(c) || c >= 0x80) return 1;
  }
  return 0;
}

static inline const char *streammax_flag_icone(const char *s) {
  if (!s) return NULL;
#define SM_EQ(x) (!strcasecmp(s, (x)))
  // Convencao de interface solicitada: TODO portugues -> Brasil;
  // TODO ingles -> EUA, inclusive pt-PT e en-GB.
  // As etiquetas originais ficam intactas para filtros e selecao de audio.
  if (SM_EQ("pt") || SM_EQ("por") || SM_EQ("pob") ||
      SM_EQ("pb") || SM_EQ("br") || SM_EQ("ptb") || SM_EQ("ptbr") ||
      !strncasecmp(s, "pt-", 3) || !strncasecmp(s, "pt_", 3) ||
      SM_EQ("Portuguese") || SM_EQ("Português") ||
      SM_EQ("Portugues") || SM_EQ("Brazilian Portuguese") ||
      SM_EQ("Portuguese Brazilian") ||
      SM_EQ("Português brasileiro") ||
      SM_EQ("Portugues brasileiro") ||
      SM_EQ("Português do Brasil") ||
      !strncasecmp(s, "Portuguese (", 12) ||
      !strncasecmp(s, "Português (", 12) ||
      !strncasecmp(s, "Portugues (", 11))
    return "sm_flag_br";
  if (SM_EQ("en") || SM_EQ("eng") || SM_EQ("English") ||
      SM_EQ("Inglês") || SM_EQ("Ingles") ||
      !strncasecmp(s, "en-", 3) || !strncasecmp(s, "en_", 3) ||
      !strncasecmp(s, "English (", 9))
    return "sm_flag_us";
  if (SM_EQ("es-ES") || SM_EQ("es_ES") || SM_EQ("Spanish (Spain)") ||
      SM_EQ("Español (España)"))
    return "sm_flag_es";
  if (SM_EQ("es-MX") || SM_EQ("es_MX") || SM_EQ("Spanish (Mexico)") ||
      SM_EQ("Español (México)"))
    return "sm_flag_mx";
#undef SM_EQ
  return NULL;
}

// Recupera SOMENTE bandeiras com idioma explicitamente declarado no campo
// audioIdiomas (CSV); sem inferir idiomas de "Dual Audio" ou de legendas.
// Retorna no maximo 'cap' icones distintos, sem alocar memoria.
static inline unsigned streammax_audio_bandeiras(const char *csv,
                                                 const char **flags,
                                                 unsigned cap) {
  unsigned n = 0;
  if (!csv || !flags || !cap) return 0;
  for (const char *p = csv; *p && n < cap;) {
    const char *fim = strchr(p, ',');
    const char *ate = fim ? fim : p + strlen(p);
    char codigo[128];
    const char *icone;
    while (p < ate && isspace((unsigned char)*p)) p++;
    while (ate > p && isspace((unsigned char)ate[-1])) ate--;
    size_t len = (size_t)(ate - p);
    icone = NULL;
    if (len && len < sizeof codigo) {
      memcpy(codigo, p, len);
      codigo[len] = 0;
      icone = streammax_flag_icone(codigo);
    }
    if (icone) {
      unsigned i;
      for (i = 0; i < n; i++)
        if (!strcmp(flags[i], icone)) break;
      if (i == n) flags[n++] = icone;
    }
    p = fim ? fim + 1 : ate;
  }
  return n;
}
#endif
