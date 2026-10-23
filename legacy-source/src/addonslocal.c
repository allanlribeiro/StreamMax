#include "addonslocal.h"
#include "sessao.h"
#include "perfis.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <inttypes.h>

/* Um arquivo por conta + perfil de addons; nunca gravar endereços contendo
 * tokens no repositório, no log, em assets do APK ou na nuvem.
 * SDL_GetPrefPath aponta para armazenamento privado do aplicativo Android. */
static int arquivo(char *dest, size_t n) {
    char *dir = SDL_GetPrefPath("StreamMax", "StreamMax");
    const unsigned char *p = (const unsigned char *)sessao_usuario();
    uint64_t h = UINT64_C(14695981039346656037);
    int perfil = perfis_ativo_addons();
    int ok;
    if (!dir || !*dir) { SDL_free(dir); return 0; }
    if (!p) p = (const unsigned char *)"";
    while (*p) { h ^= *p++; h *= UINT64_C(1099511628211); }
    ok = snprintf(dest, n, "%saddons-locais-%016" PRIx64 "-p%d.txt",
                  dir, h, perfil) > 0;
    SDL_free(dir);
    return ok && strlen(dest) < n - 1;
}

/* Limite deste MVP: URLs que cabem na entrada longa da TV; links com
 * credenciais devem permanecer somente no armazenamento privado. */
static int canonica(const char *entrada, char *saida, size_t capacidade) {
    const char *host, *fim;
    if (!entrada) return 0;
    while (*entrada == ' ' || *entrada == '\n' || *entrada == '\r') entrada++;
    if (strlen(entrada) >= NV_ADDON_URL_MAX) return 0;
    if (strncasecmp(entrada, "https://", 8) &&
        strncasecmp(entrada, "http://", 7) &&
        strncasecmp(entrada, "stremio://", 10)) return 0;
    if (strpbrk(entrada, "\t\r\n ")) return 0;
    nv_addon_base(entrada, saida, capacidade);
    if (!strncasecmp(saida, "https://", 8)) host = saida + 8;
    else if (!strncasecmp(saida, "http://", 7)) host = saida + 7;
    else return 0;
    fim = host + strcspn(host, "/?#");
    if (fim == host || memchr(host, '@', (size_t)(fim - host))) return 0;
    return 1;
}
static int mesmo(const char *a, const char *b) {
    char ca[NV_ADDON_URL_MAX], cb[NV_ADDON_URL_MAX];
    nv_addon_chave(a, ca, sizeof ca);
    nv_addon_chave(b, cb, sizeof cb);
    return !strcmp(ca, cb);
}
int addonslocal_guardar(const char *url) {
    char caminho[512], normal[NV_ADDON_URL_MAX];
    char linha[NV_ADDON_URL_MAX + 8];
    FILE *f;
    if (!canonica(url, normal, sizeof normal) || !arquivo(caminho, sizeof caminho))
        return 0;
    f = fopen(caminho, "a+");
    if (!f) return 0;
    rewind(f);
    while (fgets(linha, sizeof linha, f)) {
        size_t n = strlen(linha);
        if (n && linha[n - 1] == '\n') linha[--n] = '\0';
        if (n && mesmo(linha, normal)) { fclose(f); return 2; }
    }
    fseek(f, 0, SEEK_END);
    if (fprintf(f, "%s\n", normal) < 0 || fflush(f) != 0) {
        fclose(f);
        return 0;
    }
    if (fclose(f) != 0) return 0;
    return 1;
}
int addonslocal_combinar(const AddonRemoto *conta, int n,
                         AddonRemoto *saida, int capacidade) {
    char caminho[512], linha[NV_ADDON_URL_MAX + 8], normal[NV_ADDON_URL_MAX];
    FILE *f;
    int i, j, k = 0;
    if (!saida || capacidade <= 0) return 0;
    if (capacidade > ADDONS_MAX) capacidade = ADDONS_MAX;
    if (conta) for (i = 0; i < n && k < capacidade; i++) {
        if (!conta[i].url[0]) continue;
        saida[k++] = conta[i];
    }
    if (!arquivo(caminho, sizeof caminho)) return k;
    f = fopen(caminho, "r");
    if (!f) return k;
    while (k < capacidade && fgets(linha, sizeof linha, f)) {
        const char *h, *fim;
        size_t len = strlen(linha);
        int repetido = 0;
        /* Linhas parciais (queda de energia) ou grandes demais não entram. */
        if (!len || linha[len - 1] != '\n') {
            int x;
            while ((x = fgetc(f)) != '\n' && x != EOF) {}
            continue;
        }
        linha[len - 1] = 0;
        if (!canonica(linha, normal, sizeof normal)) continue;
        for (j = 0; j < k; j++) if (mesmo(saida[j].url, normal)) {
            repetido = 1; break;
        }
        if (repetido) continue;
        memset(&saida[k], 0, sizeof saida[k]);
        snprintf(saida[k].url, sizeof saida[k].url, "%s", normal);
        h = strstr(normal, "://");
        h = h ? h + 3 : normal;
        fim = h + strcspn(h, "/?#");
        snprintf(saida[k].nome, sizeof saida[k].nome, "Local: %.*s",
                 (int)((fim - h) < 48 ? (fim - h) : 48), h);
        saida[k].ativo = 1;
        k++;
    }
    fclose(f);
    return k;
}
