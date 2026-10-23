#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$tmp" <<'PY'
import pathlib, sys
root = pathlib.Path("src")
dest = pathlib.Path(sys.argv[1])
c = (root / "addonslocal.c").read_text(encoding="utf-8")
# Compila e executa o corpo de armazenamento real. Somente dependências
# de plataforma SDL/login são substituídas por dublês no teste host.
for include in ('"addonslocal.h"', '"sessao.h"', '"perfis.h"', '<SDL2/SDL.h>'):
    marker = '#include ' + include + '\n'
    assert c.count(marker) == 1, (include, "inclusão alterada")
    c = c.replace(marker, "", 1)
header = r"""
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <inttypes.h>
#include "addonurl.h"
#define ADDONS_MAX 128
typedef struct { char nome[64]; char url[NV_ADDON_URL_MAX]; int ativo; } AddonRemoto;
static char conta[100] = "perfil-principal";
static int perfil = 1;
static const char *sessao_usuario(void) { return conta; }
static int perfis_ativo_addons(void) { return perfil; }
static char *SDL_GetPrefPath(const char *a, const char *b) {
    const char *dir = getenv("STREAMMAX_ADDON_TEST_DIR");
    char *p;
    (void)a; (void)b;
    if (!dir) return NULL;
    p = malloc(strlen(dir) + 2);
    if (p) sprintf(p, "%s/", dir);
    return p;
}
static void SDL_free(void *p) { free(p); }
"""
driver = r"""
#include <assert.h>
int main(void) {
    AddonRemoto remotos[2] = {0};
    AddonRemoto *saida = calloc(ADDONS_MAX, sizeof *saida);
    int n;
    assert(saida);
    assert(addonslocal_guardar("javascript:evil") == 0);
    assert(addonslocal_guardar("https://") == 0);
    assert(addonslocal_guardar("https://user:pass@host/manifest.json") == 0);
    assert(addonslocal_guardar("https://addon.exemplo.com/manifest.json") == 1);
    assert(addonslocal_guardar("https://addon.exemplo.com/") == 2);
    n = addonslocal_combinar(NULL, 0, saida, ADDONS_MAX);
    assert(n == 1);
    assert(!strcmp(saida[0].url, "https://addon.exemplo.com"));
    assert(saida[0].ativo == 1);
    assert(addonslocal_contem("https://addon.exemplo.com/manifest.json") == 1);
    assert(addonslocal_ativar("https://addon.exemplo.com", 0) == 1);
    n = addonslocal_combinar(NULL, 0, saida, ADDONS_MAX);
    assert(n == 1 && saida[0].ativo == 0);
    assert(addonslocal_ativar("https://nao-registrado.com", 1) == 0);

    strcpy(remotos[0].nome, "Servidor");
    strcpy(remotos[0].url, "https://servidor.exemplo/manifest.json");
    remotos[0].ativo = 1;
    n = addonslocal_combinar(remotos, 1, saida, ADDONS_MAX);
    assert(n == 2);
    assert(!strcmp(saida[0].nome, "Servidor"));
    assert(saida[1].ativo == 0); /* não reativa addon local */

    strcpy(remotos[0].url, "https://addon.exemplo.com");
    n = addonslocal_combinar(remotos, 1, saida, ADDONS_MAX);
    assert(n == 1); /* sem duplicatas entre servidor/local */
    assert(saida[0].ativo == 1); /* servidor tem preferência */
    n = addonslocal_combinar(NULL, 0, saida, ADDONS_MAX);
    assert(n == 1 && saida[0].ativo == 0); /* estado local persiste */

    strcpy(conta, "outro-usuario");
    n = addonslocal_combinar(NULL, 0, saida, ADDONS_MAX);
    assert(n == 0);
    strcpy(conta, "perfil-principal");
    perfil = 2;
    n = addonslocal_combinar(NULL, 0, saida, ADDONS_MAX);
    assert(n == 0);
    perfil = 1;
    n = addonslocal_combinar(NULL, 0, saida, ADDONS_MAX);
    assert(n == 1 && saida[0].ativo == 0); /* persistiu por conta/perfil */
    assert(addonslocal_guardar("stremio://outro-addon.exemplo/manifest.json") == 1);
    n = addonslocal_combinar(NULL, 0, saida, ADDONS_MAX);
    assert(n == 2 && saida[1].ativo == 1);
    assert(!strcmp(saida[1].url, "https://outro-addon.exemplo"));
    assert(addonslocal_remover("https://addon.exemplo.com/manifest.json") == 1);
    assert(!addonslocal_contem("https://addon.exemplo.com"));
    n = addonslocal_combinar(NULL, 0, saida, ADDONS_MAX);
    assert(n == 1 && !strcmp(saida[0].url, "https://outro-addon.exemplo"));
    assert(addonslocal_remover("https://outro-addon.exemplo") == 1);
    assert(addonslocal_remover("https://outro-addon.exemplo") == 0);
    assert(addonslocal_combinar(NULL, 0, saida, ADDONS_MAX) == 0);
    free(saida);
    puts("OK: URL, persistência, isolamento e sincronização");
    return 0;
}
"""
(dest / "merged.c").write_text(header + c + driver, encoding="utf-8")
ui = (root / "addonsui.c").read_text(encoding="utf-8")
layout = (root / "ajustes_ux_ilha.inc").read_text(encoding="utf-8")
addons = (root / "addons.c").read_text(encoding="utf-8")
assert ui.count("void addonsui_evento(") == 1
assert 'teclado_abrir_com("Adicionar addon por URL"' in ui
assert "addonslocal_guardar(url)" in ui
assert "addonslocal_ativar(url, !addons_ativo(foco))" in ui
assert "addonslocal_remover(url)" in ui
assert "addonsui_confirmando_remocao()" in layout
assert "addons_adicionar(" in ui
assert '"Adicionar addon por URL"' in layout
assert "ajSubLista(L, n + 2, foco" in layout
assert "addonslocal_combinar(nova, n, lista, ADDONS_MAX)" in addons
print("OK: integração da UI, teclado e mesclagem verificada")
PY
cc -std=c11 -Wall -Wextra -Werror -Wno-unused-function -I src "$tmp/merged.c" -o "$tmp/test"
STREAMMAX_ADDON_TEST_DIR="$tmp" "$tmp/test"
