#include "addonmanifest.h"
#include "js.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>

/* Parser estrutural limitado. js.c continua responsável por decodificar os
 * textos; uma página HTML, JSON truncado ou um erro do servidor não é addon. */
typedef struct { const char *p, *fim; } Json;
static void espaco(Json *j) { while (j->p < j->fim && strchr(" \r\n\t", *j->p)) j->p++; }
static int texto(Json *j) {
  if (j->p >= j->fim || *j->p++ != '"') return 0;
  while (j->p < j->fim) {
    unsigned char c = (unsigned char)*j->p++;
    if (c == '"') return 1;
    if (c < 32) return 0;
    if (c == '\\') {
      if (j->p >= j->fim) return 0;
      c = (unsigned char)*j->p++;
      if (c == 'u') {
        for (int k = 0; k < 4; k++) if (j->p >= j->fim || !isxdigit((unsigned char)*j->p++)) return 0;
      } else if (!strchr("\"\\/bfnrt", c)) return 0;
    }
  }
  return 0;
}
static int valor(Json *j, int nivel) {
  char fecha;
  espaco(j);
  if (nivel > 32 || j->p >= j->fim) return 0;
  if (*j->p == '"') return texto(j);
  if (*j->p == '{' || *j->p == '[') {
    int objeto = *j->p++ == '{';
    fecha = objeto ? '}' : ']'; espaco(j);
    if (j->p < j->fim && *j->p == fecha) { j->p++; return 1; }
    for (;;) {
      if (objeto) { if (!texto(j)) return 0; espaco(j); if (j->p >= j->fim || *j->p++ != ':') return 0; }
      if (!valor(j, nivel + 1)) return 0;
      espaco(j); if (j->p >= j->fim) return 0;
      if (*j->p == fecha) { j->p++; return 1; }
      if (*j->p++ != ',') return 0;
      espaco(j);
    }
  }
  const char *literal = *j->p == 't' ? "true" : *j->p == 'f' ? "false" : *j->p == 'n' ? "null" : NULL;
  if (literal) {
    size_t n = strlen(literal);
    if ((size_t)(j->fim - j->p) < n || memcmp(j->p, literal, n)) return 0;
    j->p += n; return 1;
  }
  if (*j->p == '-') j->p++;
  if (j->p >= j->fim || !isdigit((unsigned char)*j->p)) return 0;
  if (*j->p == '0') j->p++;
  else while (j->p < j->fim && isdigit((unsigned char)*j->p)) j->p++;
  if (j->p < j->fim && *j->p == '.') {
    j->p++; if (j->p >= j->fim || !isdigit((unsigned char)*j->p)) return 0;
    while (j->p < j->fim && isdigit((unsigned char)*j->p)) j->p++;
  }
  if (j->p < j->fim && (*j->p == 'e' || *j->p == 'E')) {
    j->p++; if (j->p < j->fim && (*j->p == '+' || *j->p == '-')) j->p++;
    if (j->p >= j->fim || !isdigit((unsigned char)*j->p)) return 0;
    while (j->p < j->fim && isdigit((unsigned char)*j->p)) j->p++;
  }
  return 1;
}
/* Documento já validado. Nunca encontra chaves de objetos internos. */
static const char *campo(const char *p, const char *fim, const char *chave) {
  Json j = {p, fim}; espaco(&j);
  if (j.p >= fim || *j.p++ != '{') return NULL;
  for (;;) {
    espaco(&j); if (j.p >= fim || *j.p != '"') return NULL;
    const char *inicio = j.p + 1;
    if (!texto(&j)) return NULL;
    size_t n = (size_t)(j.p - inicio - 1);
    espaco(&j); if (j.p >= fim || *j.p++ != ':') return NULL;
    espaco(&j); const char *v = j.p;
    if (strlen(chave) == n && !memcmp(inicio, chave, n)) return v;
    if (!valor(&j, 1)) return NULL;
    espaco(&j); if (j.p >= fim || *j.p++ != ',') return NULL;
  }
}
static int preenchido(const char *p) { while (*p && isspace((unsigned char)*p)) p++; return *p != 0; }
AddonManifestoEstado addonmanifest_validar(const char *corpo, size_t n, char *nome, size_t tamanho) {
  char id[256] = "", titulo[512] = "", versao[128] = "";
  if (nome && tamanho) nome[0] = 0;
  if (!corpo || !n || n > ADDON_MANIFESTO_MAX || memchr(corpo, 0, n)) return AM_INVALIDO;
  Json j = {corpo, corpo + n}; espaco(&j);
  if (j.p >= j.fim || *j.p != '{' || !valor(&j, 0)) return AM_INVALIDO;
  espaco(&j); if (j.p != j.fim) return AM_INVALIDO;
  if (!js_texto_raiz(corpo, "id", id, sizeof id) || !preenchido(id) ||
      !js_texto_raiz(corpo, "name", titulo, sizeof titulo) || !preenchido(titulo) ||
      !js_texto_raiz(corpo, "version", versao, sizeof versao) || !preenchido(versao)) return AM_INVALIDO;
  const char *r = campo(corpo, corpo + n, "resources");
  if (!r || *r != '[') return AM_INVALIDO;
  const char *h = campo(corpo, corpo + n, "behaviorHints");
  if (h && *h == '{') {
    const char *config = campo(h, js_fim(h), "configurationRequired");
    if (config && !strncmp(config, "true", 4)) return AM_CONFIGURAR;
  }
  if (nome && tamanho) {
    snprintf(nome, tamanho, "%s", titulo);
    /* Não deixar um caractere UTF-8 pela metade ao caber no rótulo da lista. */
    size_t k = strlen(nome);
    if (strlen(titulo) >= tamanho) {
      while (k && ((unsigned char)nome[k - 1] & 0xc0) == 0x80) k--;
      if (k && (unsigned char)nome[k - 1] >= 0xc0) k--;
      nome[k] = 0;
    }
  }
  return AM_OK;
}
