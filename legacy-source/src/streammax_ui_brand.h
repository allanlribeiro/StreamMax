#ifndef STREAMMAX_UI_BRAND_H
#define STREAMMAX_UI_BRAND_H

#include <stddef.h>
#include <string.h>

/* Somente textos desenhados na interface. NAO aplicar a URLs, ids, JSON,
 * nomes internos, protocolos de login ou valores persistidos. */
static inline int streammax_marca_igual(const char *s) {
  const char *alvo = "nuvio";
  int i;
  for (i = 0; i < 5; i++) {
    unsigned char c = (unsigned char)s[i];
    if (!c) return 0;
    if (c >= 'A' && c <= 'Z') c = (unsigned char)(c + ('a' - 'A'));
    if (c != (unsigned char)alvo[i]) return 0;
  }
  return 1;
}

/* A renderizacao acontece no fio principal. O anel permite varias chamadas
 * aninhadas sem sobrescrever imediatamente o texto anterior. Sem alocacao. */
static inline const char *streammax_nome_visivel(const char *s) {
  static char buffers[8][4096];
  static unsigned proximo;
  const char *p, *repl;
  char *saida;
  size_t pos = 0, tam;
  if (!s) return s;
  for (p = s; *p && !streammax_marca_igual(p); p++) {}
  if (!*p) return s;
  saida = buffers[(proximo++) % 8];
  p = s;
  while (*p) {
    if (streammax_marca_igual(p)) {
      repl = p[0] >= 'A' && p[0] <= 'Z'
        ? (p[1] >= 'A' && p[1] <= 'Z' ? "STREAMMAX" : "StreamMax")
        : "streammax";
      tam = strlen(repl);
      if (pos + tam >= sizeof buffers[0]) return s;
      memcpy(saida + pos, repl, tam);
      pos += tam;
      p += 5;
    } else {
      if (pos + 1 >= sizeof buffers[0]) return s;
      saida[pos++] = *p++;
    }
  }
  saida[pos] = 0;
  return saida;
}

#endif
