// Atualizador Android do StreamMax.
// A versao comercial e independente da versao interna herdada do Legacy.
// RC1 #146: StreamMax 1.0.0, Android 2.0.7 (20007).
// RC1 148 aprovada no teste inicial: StreamMax 1.0.1, Android 2.0.8 (20008).
// StreamMax 1.0.2 aprovado no teste OTA da TCL: Android 2.0.9 (20009).
// StreamMax 1.0.3 publicada: Android 2.0.10 (20010).
// StreamMax 1.0.4 publicada: Android 2.0.11 (20011).
// Candidato experimental StreamMax 1.0.5: Android 2.0.12 (20012), inicio informado e startup rapido.
// Candidato StreamMax 1.0.7: Android 2.0.14 (20014), Home + menu Fontes 4K.
// Alterar AMBAS as versoes a cada release: este define a versao que a
// interface compara; deploy/app/appinfo.json define o versionCode do Android.
#ifndef SM_UPDATE_H
#define SM_UPDATE_H

#include <stdio.h>
#include <string.h>

#define SM_UPDATE_VERSION "1.0.24"
#define SM_UPDATE_API  "https://api.github.com/repos/allanlribeiro/StreamMax/releases/latest"
#define SM_UPDATE_PAGE "https://github.com/allanlribeiro/StreamMax/releases"
#define SM_UPDATE_DOWNLOAD "https://github.com/allanlribeiro/StreamMax/releases/download/"

// Somente tags vX.Y.Z: sem beta, sem sufixos ou campos negativos.
static int sm_update_tripla(const char *s, unsigned long v[3]) {
  int i;
  if (!s) return 0;
  for (i = 0; i < 3; i++) {
    unsigned long n = 0;
    if (*s < '0' || *s > '9') return 0;
    while (*s >= '0' && *s <= '9') {
      n = n * 10ul + (unsigned long)(*s++ - '0');
      if (n > 9999ul) return 0;
    }
    v[i] = n;
    if (i < 2) { if (*s++ != '.') return 0; }
  }
  return !*s;
}
static int sm_update_tag_valida(const char *tag) {
  unsigned long v[3];
  return tag && *tag == 'v' && sm_update_tripla(tag + 1, v);
}
static int sm_update_nova_versao(const char *tag) {
  unsigned long remoto[3], atual[3];
  int i;
  if (!sm_update_tag_valida(tag) ||
      !sm_update_tripla(SM_UPDATE_VERSION, atual) ||
      !sm_update_tripla(tag + 1, remoto)) return 0;
  for (i = 0; i < 3; i++) {
    if (remoto[i] != atual[i]) return remoto[i] > atual[i];
  }
  return 0;
}
// Apenas o arquivo oficial exatamente correspondente a tag da release.
// O instalador Android ainda exige mesmo packageName, assinatura e codigo
// superior, mesmo que a origem do GitHub seja comprometida.
static int sm_update_url_apk_valida(const char *tag, const char *url) {
  char esperado[512];
  int n;
  if (!sm_update_tag_valida(tag) || !url) return 0;
  n = snprintf(esperado, sizeof esperado,
               SM_UPDATE_DOWNLOAD "%s/StreamMax-%s-android.apk",
               tag, tag + 1);
  return n > 0 && (size_t)n < sizeof esperado && !strcmp(esperado, url);
}
#endif
