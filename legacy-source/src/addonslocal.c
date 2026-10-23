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
#include <ctype.h>

/* Apenas ficheiros privados do aplicativo. URLs de addons podem conter
 * credenciais; nunca exibir o caminho completo em logs ou mensagens. */
typedef struct {
  char url[NV_ADDON_URL_MAX];
  int ativo;
} LocalAddon;

static int arquivo(char *dst, size_t n) {
  char *dir = SDL_GetPrefPath("StreamMax", "StreamMax");
  const unsigned char *p = (const unsigned char *)sessao_usuario();
  uint64_t hash = UINT64_C(14695981039346656037);
  int escrito;
  if (!dir) return 0;
  if (!p) p = (const unsigned char *)"";
  while (*p) { hash ^= *p++; hash *= UINT64_C(1099511628211); }
  escrito = snprintf(dst, n, "%saddons-locais-%016" PRIx64 "-p%d.txt",
                     dir, hash, perfis_ativo_addons());
  SDL_free(dir);
  return escrito > 0 && (size_t)escrito < n;
}

static int normalizar(const char *entrada, char *dst, size_t n) {
  const char *host, *fim;
  if (!entrada) return 0;
  while (*entrada && isspace((unsigned char)*entrada)) entrada++;
  if (strlen(entrada) >= NV_ADDON_URL_MAX) return 0;
  if (strncasecmp(entrada, "https://", 8) &&
      strncasecmp(entrada, "http://", 7) &&
      strncasecmp(entrada, "stremio://", 10)) return 0;
  if (strpbrk(entrada, " \t\r\n")) return 0;
  nv_addon_base(entrada, dst, n);
  if (!strncasecmp(dst, "https://", 8)) host = dst + 8;
  else if (!strncasecmp(dst, "http://", 7)) host = dst + 7;
  else return 0;
  fim = host + strcspn(host, "/?#");
  /* Não aceitar host vazio nem user:senha no endereço. */
  if (fim == host || memchr(host, '@', (size_t)(fim - host))) return 0;
  return 1;
}

static int iguais(const char *a, const char *b) {
  char x[NV_ADDON_URL_MAX], y[NV_ADDON_URL_MAX];
  nv_addon_chave(a, x, sizeof x);
  nv_addon_chave(b, y, sizeof y);
  return !strcmp(x, y);
}

static int ler(LocalAddon *itens, int capacidade, const char *caminho) {
  FILE *f = fopen(caminho, "r");
  char linha[NV_ADDON_URL_MAX + 16];
  int n = 0;
  if (!f) return 0;
  while (n < capacidade && fgets(linha, sizeof linha, f)) {
    char normal[NV_ADDON_URL_MAX];
    char *url = linha;
    int ativo = 1, i, repetido = 0;
    size_t len = strlen(linha);
    if (!len || linha[len - 1] != '\n') {
      int x;
      while ((x = fgetc(f)) != '\n' && x != EOF) {}
      continue;
    }
    linha[len - 1] = 0;
    /* Arquivos antigos têm apenas a URL. Novo formato: 0<TAB>URL. */
    if (url[0] == '0' && url[1] == '\t') { ativo = 0; url += 2; }
    else if (url[0] == '1' && url[1] == '\t') url += 2;
    if (!normalizar(url, normal, sizeof normal)) continue;
    for (i = 0; i < n; i++) if (iguais(itens[i].url, normal)) {
      repetido = 1; break;
    }
    if (repetido) continue;
    snprintf(itens[n].url, sizeof itens[n].url, "%s", normal);
    itens[n].ativo = ativo;
    n++;
  }
  fclose(f);
  return n;
}

/* Salvar por substituição atômica: o sincronismo não observa arquivo truncado.
 * A mesma conta/perfil é usada em todas as operações da interface. */
static int escrever(const LocalAddon *itens, int n, const char *caminho) {
  char tmp[600];
  FILE *f;
  int i, erro = 0;
  int k = snprintf(tmp, sizeof tmp, "%s.tmp", caminho);
  if (k <= 0 || (size_t)k >= sizeof tmp) return 0;
  f = fopen(tmp, "w");
  if (!f) return 0;
  for (i = 0; i < n; i++)
    if (fprintf(f, "%d\t%s\n", !!itens[i].ativo, itens[i].url) < 0) erro = 1;
  if (fflush(f) != 0) erro = 1;
  if (fclose(f) != 0) erro = 1;
  if (!erro && rename(tmp, caminho) == 0) return 1;
  remove(tmp);
  return 0;
}

static LocalAddon *criarLista(void) {
  return calloc(ADDONS_MAX, sizeof(LocalAddon));
}

int addonslocal_guardar(const char *url) {
  char caminho[512], normal[NV_ADDON_URL_MAX];
  LocalAddon *itens;
  int n, i, r = 0;
  if (!normalizar(url, normal, sizeof normal) || !arquivo(caminho, sizeof caminho))
    return 0;
  itens = criarLista();
  if (!itens) return 0;
  n = ler(itens, ADDONS_MAX, caminho);
  for (i = 0; i < n; i++) if (iguais(itens[i].url, normal)) {
    r = 2; goto fim;
  }
  if (n >= ADDONS_MAX) goto fim;
  snprintf(itens[n].url, sizeof itens[n].url, "%s", normal);
  itens[n].ativo = 1;
  r = escrever(itens, n + 1, caminho) ? 1 : 0;
fim:
  free(itens);
  return r;
}

int addonslocal_contem(const char *url) {
  char caminho[512], normal[NV_ADDON_URL_MAX];
  LocalAddon *itens;
  int n, i, achou = 0;
  if (!normalizar(url, normal, sizeof normal) || !arquivo(caminho, sizeof caminho))
    return 0;
  itens = criarLista();
  if (!itens) return 0;
  n = ler(itens, ADDONS_MAX, caminho);
  for (i = 0; i < n; i++) if (iguais(itens[i].url, normal)) { achou = 1; break; }
  free(itens);
  return achou;
}

int addonslocal_ativar(const char *url, int ativo) {
  char caminho[512], normal[NV_ADDON_URL_MAX];
  LocalAddon *itens;
  int n, i, ok = 0;
  if (!normalizar(url, normal, sizeof normal) || !arquivo(caminho, sizeof caminho))
    return 0;
  itens = criarLista();
  if (!itens) return 0;
  n = ler(itens, ADDONS_MAX, caminho);
  for (i = 0; i < n; i++) if (iguais(itens[i].url, normal)) {
    if (!!itens[i].ativo == !!ativo) ok = 1;
    else {
      itens[i].ativo = !!ativo;
      ok = escrever(itens, n, caminho);
    }
    break;
  }
  free(itens);
  return ok;
}

int addonslocal_remover(const char *url) {
  char caminho[512], normal[NV_ADDON_URL_MAX];
  LocalAddon *itens;
  int n, i, ok = 0;
  if (!normalizar(url, normal, sizeof normal) || !arquivo(caminho, sizeof caminho))
    return 0;
  itens = criarLista();
  if (!itens) return 0;
  n = ler(itens, ADDONS_MAX, caminho);
  for (i = 0; i < n; i++) if (iguais(itens[i].url, normal)) {
    if (i < n - 1)
      memmove(&itens[i], &itens[i + 1], (size_t)(n - i - 1) * sizeof *itens);
    ok = escrever(itens, n - 1, caminho);
    break;
  }
  free(itens);
  return ok;
}

int addonslocal_combinar(const AddonRemoto *conta, int n,
                         AddonRemoto *saida, int capacidade) {
  char caminho[512];
  LocalAddon *itens;
  int k = 0, nLocais, i, j;
  if (!saida || capacidade <= 0) return 0;
  if (capacidade > ADDONS_MAX) capacidade = ADDONS_MAX;
  if (conta) for (i = 0; i < n && k < capacidade; i++) {
    if (conta[i].url[0]) saida[k++] = conta[i];
  }
  if (!arquivo(caminho, sizeof caminho)) return k;
  itens = criarLista();
  if (!itens) return k;
  nLocais = ler(itens, ADDONS_MAX, caminho);
  for (i = 0; i < nLocais && k < capacidade; i++) {
    const char *host, *fim;
    int duplicado = 0;
    for (j = 0; j < k; j++) if (iguais(saida[j].url, itens[i].url)) {
      duplicado = 1; break;
    }
    if (duplicado) continue;
    memset(&saida[k], 0, sizeof saida[k]);
    snprintf(saida[k].url, sizeof saida[k].url, "%s", itens[i].url);
    host = strstr(itens[i].url, "://");
    host = host ? host + 3 : itens[i].url;
    fim = host + strcspn(host, "/?#");
    snprintf(saida[k].nome, sizeof saida[k].nome, "Local: %.*s",
             (int)((fim - host) < 48 ? (fim - host) : 48), host);
    saida[k].ativo = itens[i].ativo;
    k++;
  }
  free(itens);
  return k;
}
