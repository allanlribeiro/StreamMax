#include "trailerhls.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static int atributo(const char *linha, const char *nome, char *dst, size_t cap) {
  size_t n = strlen(nome), k = 0;
  const char *p = linha;
  while ((p = strstr(p, nome))) {
    if ((p == linha || p[-1] == ',' || p[-1] == ':') && p[n] == '=') {
      p += n + 1;
      if (*p == '"') { p++; while (*p && *p != '"' && k + 1 < cap) dst[k++] = *p++; }
      else while (*p && *p != ',' && *p != '\r' && *p != '\n' && k + 1 < cap) dst[k++] = *p++;
      dst[k] = 0;
      return 1;
    }
    p += n;
  }
  dst[0] = 0;
  return 0;
}

static int portugues(const char *s) {
  if (!strcasecmp(s, "pt-BR") || !strcasecmp(s, "pt_BR")) return 30;
  if (!strcasecmp(s, "pt")) return 20;
  if (!strncasecmp(s, "pt-", 3) || !strncasecmp(s, "pt_", 3)) return 10;
  return 0;
}

static void tirarSubtitulos(char *inf) {
  char *p = strstr(inf, ",SUBTITLES=");
  char *fim;
  if (!p) return;
  fim = p + strlen(",SUBTITLES=");
  if (*fim == '"') { fim++; while (*fim && *fim != '"') fim++; if (*fim) fim++; }
  else while (*fim && *fim != ',' && *fim != '\r' && *fim != '\n') fim++;
  memmove(p, fim, strlen(fim) + 1);
}

int sm_trailer_hls_reduzir(FILE *master, FILE *saida, int teto, int *largura) {
  char linha[4096], inf[4096] = "", uri[2048] = "";
  char audio[256] = "", subs[256] = "";
  char melhorAudio[4096] = "", melhorSub[4096] = "";
  int wMelhor = 0, avcMelhor = 0, pesoAudio = -1, pesoSub = 0;
  int limite = teto > 0 && teto <= 480 ? 864 : teto > 0 && teto <= 720 ? 1280 : 1920;
  if (!master || !saida) return 0;
  rewind(master);
  while (fgets(linha, sizeof linha, master)) {
    char res[40], codecs[256], u[2048];
    int w, avc;
    if (strncmp(linha, "#EXT-X-STREAM-INF:", 18)) continue;
    atributo(linha, "RESOLUTION", res, sizeof res);
    atributo(linha, "CODECS", codecs, sizeof codecs);
    w = atoi(res); avc = strstr(codecs, "avc1") != NULL;
    if (!fgets(u, sizeof u, master)) break;
    u[strcspn(u, "\r\n")] = 0;
    if (w <= 0 || w > limite || strncmp(u, "https://", 8) ||
        strstr(codecs, "dvh1") || strstr(codecs, "dvhe")) continue;
    if (w > wMelhor || (w == wMelhor && avc && !avcMelhor)) {
      wMelhor = w; avcMelhor = avc;
      snprintf(inf, sizeof inf, "%s", linha);
      snprintf(uri, sizeof uri, "%s", u);
      atributo(linha, "AUDIO", audio, sizeof audio);
      atributo(linha, "SUBTITLES", subs, sizeof subs);
    }
  }
  if (!wMelhor) return 0;
  rewind(master);
  while (fgets(linha, sizeof linha, master)) {
    char tipo[32], grupo[256], lingua[48], u[2048];
    int peso;
    if (strncmp(linha, "#EXT-X-MEDIA:", 13)) continue;
    atributo(linha, "TYPE", tipo, sizeof tipo);
    atributo(linha, "GROUP-ID", grupo, sizeof grupo);
    atributo(linha, "LANGUAGE", lingua, sizeof lingua);
    atributo(linha, "URI", u, sizeof u);
    if (strncmp(u, "https://", 8)) continue;
    peso = portugues(lingua) * 10;
    /* Prefer regular dialogue to audio description / forced-only captions. */
    if (!strstr(linha, "public.accessibility.describes-video")) peso += 2;
    if (!strcmp(tipo, "AUDIO") && audio[0] && !strcmp(grupo, audio)) {
      if (strstr(linha, "DEFAULT=YES")) peso++;
      if (peso > pesoAudio) { pesoAudio = peso; snprintf(melhorAudio, sizeof melhorAudio, "%s", linha); }
    } else if (!strcmp(tipo, "SUBTITLES") && subs[0] && !strcmp(grupo, subs) && portugues(lingua)) {
      if (!strstr(linha, "FORCED=YES")) peso += 2;
      if (peso > pesoSub) { pesoSub = peso; snprintf(melhorSub, sizeof melhorSub, "%s", linha); }
    }
  }
  if (audio[0] && !melhorAudio[0]) return 0;  // Never silently lose the audio group.
  if (!melhorSub[0]) tirarSubtitulos(inf);
  fputs("#EXTM3U\n#EXT-X-VERSION:6\n#EXT-X-INDEPENDENT-SEGMENTS\n", saida);
  if (melhorAudio[0]) fputs(melhorAudio, saida);
  if (melhorSub[0]) fputs(melhorSub, saida);
  fputs(inf, saida); fprintf(saida, "%s\n", uri);
  if (largura) *largura = wMelhor;
  return !ferror(saida);
}
