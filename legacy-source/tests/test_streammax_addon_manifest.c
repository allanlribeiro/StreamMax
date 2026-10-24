#include "addonmanifest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define VALIDO "{\"id\":\"org.test\",\"name\":\"Addon de teste\",\"version\":\"1.0.0\",\"resources\":[\"stream\"]}"
static int testar(const char *p) { char nome[64]; return addonmanifest_validar(p, strlen(p), nome, sizeof nome); }
int main(void) {
  char nome[64], profundo[1024];
  assert(addonmanifest_validar(VALIDO, strlen(VALIDO), nome, sizeof nome) == AM_OK);
  assert(!strcmp(nome, "Addon de teste"));
  assert(testar("  " VALIDO "\n") == AM_OK);
  assert(testar("<html>Configurar addon</html>") == AM_INVALIDO);
  assert(testar("{\"error\":\"token inválido\"}") == AM_INVALIDO);
  assert(testar("{\"id\":\"x\",\"name\":\"A\",\"version\":\"1\",\"resources\":[]") == AM_INVALIDO);
  assert(testar(VALIDO " outro texto") == AM_INVALIDO);
  assert(testar("{\"id\":\"x\" \"name\":\"A\",\"version\":\"1\",\"resources\":[]}") == AM_INVALIDO);
  assert(testar("{\"id\":\"x\",\"name\":\"A\",\"version\":\"1\",\"resources\":[],}") == AM_INVALIDO);
  assert(testar("{\"catalogs\":[{\"id\":\"interno\"}],\"name\":\"A\",\"version\":\"1\",\"resources\":[]}") == AM_INVALIDO);
  assert(testar("{\"id\":\"x\",\"name\":null,\"version\":\"1\",\"resources\":[]}") == AM_INVALIDO);
  assert(testar("{\"id\":\"  \",\"name\":\"A\",\"version\":\"1\",\"resources\":[]}") == AM_INVALIDO);
  assert(testar("{\"id\":\"x\",\"name\":\"A\",\"version\":1,\"resources\":[]}") == AM_INVALIDO);
  assert(testar("{\"id\":\"x\",\"name\":\"A\",\"version\":\"1\",\"resources\":{}}") == AM_INVALIDO);
  assert(testar("{\"id\":\"x\",\"name\":\"A\",\"version\":\"1\",\"resources\":[\"stream\"],\"behaviorHints\":{\"configurationRequired\":true}}") == AM_CONFIGURAR);
  assert(testar("{\"id\":\"x\",\"name\":\"A\",\"version\":\"1\",\"resources\":[{\"name\":\"stream\"}],\"behaviorHints\":{\"configurationRequired\":false}}") == AM_OK);
  assert(testar("{\"id\":\"x\",\"name\":\"A\\q\",\"version\":\"1\",\"resources\":[]}") == AM_INVALIDO);
  assert(testar("{\"id\":\"x\",\"name\":\"A\",\"version\":\"1\",\"resources\":[],\"x\":01}") == AM_INVALIDO);
  snprintf(profundo, sizeof profundo, "{\"id\":\"x\",\"name\":\"A\",\"version\":\"1\",\"resources\":[],\"x\":");
  for (int i = 0; i < 40; i++) strcat(profundo, "[");
  strcat(profundo, "0"); for (int i = 0; i < 40; i++) strcat(profundo, "]"); strcat(profundo, "}");
  assert(testar(profundo) == AM_INVALIDO);
  assert(addonmanifest_validar(VALIDO, ADDON_MANIFESTO_MAX + 1, nome, sizeof nome) == AM_INVALIDO);
  puts("OK: 19 cenários de manifesto, configuração, tipos, limites e JSON truncado");
  return 0;
}
