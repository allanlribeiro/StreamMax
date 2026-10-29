#include "legfala.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

void legfala_frase(const LegendaCue *cue, char *dst, size_t tam) {
  const char *p = cue ? cue->texto : "";
  size_t n = 0;
  if (!dst || !tam) return;
  while (*p && n + 1 < tam) {
    if (p[0] == '{' && p[1] == '\\') {
      const char *end = strchr(p, '}');
      if (end) { p = end + 1; continue; }
    }
    if (p[0] == '\\' && (p[1] == 'N' || p[1] == 'n')) { dst[n++] = '\n'; p += 2; }
    else dst[n++] = *p++;
  }
  // Never terminate a displayed UTF-8 character in the middle on truncation.
  if (*p) while (n && ((unsigned char)*p & 0xc0) == 0x80) { n--; p--; }
  dst[n] = 0;
}

void legfala_fechar(LegFala *f) {
  if (!f) return;
  legenda_documento_liberar(f->doc); free(f->indices); memset(f, 0, sizeof *f);
}

int legfala_abrir(LegFala *f, LegendaDocumento *doc, unsigned geracao) {
  if (!f) return 0;
  doc = legenda_documento_reter(doc); // tambem permite reabrir o mesmo documento
  int n = 0;
  const LegendaDocumentoInfo *info = legenda_documento_info(doc);
  const LegendaCue *v = legenda_documento_dados(doc, &n);
  legfala_fechar(f);
  if (!info || !(info->flags & LEGENDA_DOC_COMPLETO) ||
      (info->flags & LEGENDA_DOC_JANELA) || !v || n < 1) {
    legenda_documento_liberar(doc); return 0;
  }
  f->indices = malloc((size_t)n * sizeof *f->indices);
  if (!f->indices) { legenda_documento_liberar(doc); return 0; }
  for (int i = 0; i < n; i++) {
    char texto[sizeof v[i].texto];
    const char *s = texto;
    legfala_frase(v + i, texto, sizeof texto);
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s) f->indices[f->n++] = i;
  }
  if (!f->n) { legfala_fechar(f); legenda_documento_liberar(doc); return 0; }
  f->doc = doc; f->geracao = geracao;
  return 1;
}

const LegendaCue *legfala_cue(const LegFala *f, int indice) {
  int n = 0;
  const LegendaCue *v;
  if (!f || indice < 0 || indice >= f->n) return NULL;
  v = legenda_documento_dados(f->doc, &n);
  return v && f->indices[indice] >= 0 && f->indices[indice] < n ? v + f->indices[indice] : NULL;
}

int legfala_capturar(LegFala *f, double videoMs, double legendaMs) {
  if (!f || !f->n || !isfinite(videoMs) || !isfinite(legendaMs) || videoMs < 0) return 0;
  int lo = 0, hi = f->n;
  while (lo < hi) {
    int mid = lo + (hi - lo) / 2;
    if (legfala_cue(f, mid)->inicio * 1000 < legendaMs) lo = mid + 1;
    else hi = mid;
  }
  if (lo >= f->n) lo = f->n - 1;
  if (lo > 0 && fabs(legfala_cue(f, lo-1)->inicio * 1000 - legendaMs) <=
                fabs(legfala_cue(f, lo)->inicio * 1000 - legendaMs)) lo--;
  f->foco = lo; f->capturaMs = videoMs; f->capturada = 1;
  return 1;
}

int legfala_mover(LegFala *f, int passo) {
  if (!f || !f->n) return 0;
  long long next = (long long)f->foco + passo;
  if (next < 0) next = 0;
  if (next >= f->n) next = f->n - 1;
  int changed = f->foco != (int)next; f->foco = (int)next;
  return changed;
}

int legfala_calcular(const LegFala *f, int *atrasoMs) {
  const LegendaCue *cue;
  if (!f || !f->capturada || !atrasoMs || !(cue = legfala_cue(f, f->foco))) return 0;
  double delay = f->capturaMs - cue->inicio * 1000 - LEG_FALA_REACAO_MS;
  if (!isfinite(delay) || fabs(delay) > LEG_ATRASO_MANUAL_MAX) return 0;
  *atrasoMs = (int)lround(delay); return 1;
}
