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
  // EN generico usa 🇺🇸 por convencao visual escolhida pelo usuario.
  // PT e ES genericos continuam como texto; regioes explicitas tem prioridade.
  if (SM_EQ("pt-BR") || SM_EQ("pt_BR") || SM_EQ("ptbr") ||
      SM_EQ("pob") || SM_EQ("br") || SM_EQ("pb") || SM_EQ("ptb") ||
      SM_EQ("Brazilian Portuguese") || SM_EQ("Portuguese Brazilian") ||
      SM_EQ("Portuguese (Brazil)") || SM_EQ("Portuguese (Brasil)") ||
      SM_EQ("Portuguese (BR)") || SM_EQ("Portugues brasileiro") ||
      SM_EQ("Português brasileiro") || SM_EQ("Português do Brasil") ||
      SM_EQ("Portuguese (brasil)") || SM_EQ("Português (Brasil)"))
    return "sm_flag_br";
  if (SM_EQ("pt-PT") || SM_EQ("pt_PT") ||
      SM_EQ("Portuguese (Portugal)") || SM_EQ("Portuguese (PT)") ||
      SM_EQ("Português (Portugal)") || SM_EQ("Português (PT)"))
    return "sm_flag_pt";
  if (SM_EQ("en") || SM_EQ("eng") || SM_EQ("English") ||
      SM_EQ("en-US") || SM_EQ("en_US") || SM_EQ("English (US)") ||
      SM_EQ("English (USA)") || SM_EQ("English (United States)"))
    return "sm_flag_us";
  if (SM_EQ("en-GB") || SM_EQ("en_GB") || SM_EQ("en-UK") ||
      SM_EQ("English (UK)") || SM_EQ("English (GB)") ||
      SM_EQ("English (United Kingdom)"))
    return "sm_flag_gb";
  if (SM_EQ("es-ES") || SM_EQ("es_ES") || SM_EQ("Spanish (Spain)") ||
      SM_EQ("Español (España)"))
    return "sm_flag_es";
  if (SM_EQ("es-MX") || SM_EQ("es_MX") || SM_EQ("Spanish (Mexico)") ||
      SM_EQ("Español (México)"))
    return "sm_flag_mx";
#undef SM_EQ
  return NULL;
}
#endif
