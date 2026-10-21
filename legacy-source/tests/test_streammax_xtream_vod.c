// Regras puras da lista VOD. Rede somente quando um grupo e aberto.
#include "../src/xtream.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
  assert(xtream_vod_categoria_valida(NULL));
  assert(xtream_vod_categoria_valida(""));
  assert(xtream_vod_categoria_valida("1"));
  assert(xtream_vod_categoria_valida("0000123456"));
  assert(xtream_vod_categoria_valida("12345678901234567890123"));
  assert(!xtream_vod_categoria_valida("123456789012345678901234"));
  assert(!xtream_vod_categoria_valida("../live"));
  assert(!xtream_vod_categoria_valida("2&action=get_live_streams"));
  assert(!xtream_vod_categoria_valida("4 5"));
  assert(!xtream_vod_categoria_valida("-1"));
  assert(!xtream_vod_categoria_valida("abc"));
  puts("OK: categorias Xtream validadas antes da consulta sob demanda");
  return 0;
}
