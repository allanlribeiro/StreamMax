#include "addonsui.h"
#include "idioma.h"
#include "descoberta.h"
#include "addons.h"
#include "addonslocal.h"
#include "addoninst.h"
#include "perfisessao.h"
#include "teclado.h"
#include "gfx.h"
#include "text.h"
#include "layout.h"
#include "anim.h"
#include "sync.h"
#include "ajustes.h"
#include "ilha.h"
#include "sessao.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINHA_H   96.0f
#define LINHA_GAP  8.0f
#define LISTA_X   ajustes_conteudo_x()
#define LISTA_W  1120.0f
#define TOPO     (NV_MARGEM_Y + 118.0f)
#define BASE     (NV_TELA_H - NV_MARGEM_Y - 48.0f)
#define RAIO       0.13f

static int   foco, sair, pediuRelogio, aguardandoUrl, validandoUrl;
static uint64_t sessaoCadastro;
static char  confirmarRemocao[NV_ADDON_URL_MAX];
// 1 quando alguem ligou ou desligou algum addon nesta visita. Ver a nota em
// addonsui_quer_sair.
static int   mexeu;
static float scrollY;
static float animFoco[128];

void addonsui_abrir(void) {
  addoninst_cancelar(); validandoUrl = 0;
  foco = 0; sair = 0; pediuRelogio = 0; mexeu = 0; aguardandoUrl = 0; confirmarRemocao[0] = 0; scrollY = 0.0f;
  // Sonda aqui e nao no arranque: sao N viagens de rede, e elas so interessam
  // a quem abriu esta tela. Quem ja respondeu nao e consultado de novo.
  addons_sondar_manifestos();
}

// LIGAR UM ADDON SO VALIA NA PROXIMA ABERTURA DO APP, e o relato foi esse:
// "se eu tiver com os addons desativados e ativar, eles nao ativam; tenho que
// fechar o app e abrir de novo".
//
// addons_alternar muda a lista em memoria; a mudanca e so DESTA TV (#360 mao
// unica): o proximo pull traz a lista da conta por cima. Nenhum dos dois refaz
// a descoberta. Sem um ciclo novo, os manifestos nao sao lidos de novo e a
// home continua com os catalogos do conjunto antigo. desc_repetir() e quem
// refaz o ciclo inteiro, que e o que esta troca exige de verdade: ler
// manifesto de addon que acabou de entrar nao e reordenar fileira.
//
// NA SAIDA DA TELA, e nao a cada tecla: quem liga cinco addons de uma vez paga
// um ciclo e nao cinco. O ciclo leva ~20 s nesta TV, entao a diferenca nao e
// teorica.
int addonsui_quer_sair(void) {
  if (sair && mexeu) { mexeu = 0; desc_tirar_fileiras_de_addons_ausentes(); desc_repetir(); }
  return sair;
}

int addonsui_pediu_relogio(void) { int v = pediuRelogio; pediuRelogio = 0; return v; }
int addonsui_confirmando_remocao(void) { return confirmarRemocao[0] != 0; }

// SINCRONIZAR ADDONS (#360): a ultima linha da lista (foco == n). Pede a volta
// da conta ja, pelo mesmo ciclo de sempre (sync.c, fora do fio do quadro); o
// resultado vira aviso da ilha (ilhasinais.c). Sem conta nao ha o que puxar: a
// linha explica em vez de sumir.
static void sincronizar(void) {
  if (!sessao_logada()) {
    ilha_avisar("sync-addons", ILHA_INFO, "aj_rotate-cw",
                i18n("Entre na conta Nuvio para sincronizar os addons."), 4000u, 0);
    return;
  }
  if (sync_manual_addons_rodando()) return;   // ja no ar: o botao nao empilha
  sync_manual_addons();
}

static void adicionarPorUrl(void) {
  if (validandoUrl) return;
  sessaoCadastro = nv_perfil_sessao_ler();
  /* Reutiliza a modal de texto do app, com IME Android e QR para digitar. */
  const char *alfabeto = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:/?&=._-%~@+#[]!$'()*+,;";
  teclado_contexto("Fontes e addons");
  teclado_abrir_com("Adicionar addon por URL",
                    "Digite ou cole a URL do manifesto do addon.", TECLADO_LONGO,
                    alfabeto, NULL);
  aguardandoUrl = 1;
}
/* Índices da lista: 0 = Adicionar; 1 = Sincronizar; 2... = addons.
 * Os atalhos aparecem primeiro mesmo para quem tem dezenas de addons. */
static int indiceAddon(int foco, int n) {
  int idx = foco - 2;
  return idx >= 0 && idx < n ? idx : -1;
}
/* Remover apenas a instalação local, mantendo a lista da conta. Reconstrói
 * após alterar o ficheiro privado, para não deixar o addon no estado atual. */
static void removerLocalConfirmado(void) {
  AddonRemoto *todos;
  char url[NV_ADDON_URL_MAX];
  int n, i, k;
  snprintf(url, sizeof url, "%s", confirmarRemocao);
  confirmarRemocao[0] = 0;
  todos = calloc(ADDONS_MAX, sizeof *todos);
  if (!todos) return;
  n = addons_exportar(todos, ADDONS_MAX);
  if (!addonslocal_remover(url)) {
    ilha_avisar("addon-remocao-falhou", ILHA_INFO, "aj_puzzle",
                "Não foi possível remover o addon local.", 4000u, 0);
    free(todos);
    return;
  }
  for (i = 0, k = 0; i < n; i++) {
    char a[NV_ADDON_URL_MAX], b[NV_ADDON_URL_MAX];
    nv_addon_chave(todos[i].url, a, sizeof a);
    nv_addon_chave(url, b, sizeof b);
    if (strcmp(a, b)) todos[k++] = todos[i];
  }
  if (k > 0) addons_definir_lista(todos, k);
  else addons_esquecer();
  free(todos);
  mexeu = 1;
  foco = 0;
  ilha_avisar("addon-removido", ILHA_INFO, "aj_puzzle",
              "Addon local removido desta TV.", 4000u, 0);
}

void addonsui_evento(const SDL_Event *e) {
  if (aguardandoUrl && teclado_aberto()) {
    teclado_evento(e);
    return;
  }
  int n = addons_n();
  pediuRelogio = 0;
  if (e->type != SDL_KEYDOWN) return;
  if (confirmarRemocao[0]) {
    switch (e->key.keysym.sym) {
      case SDLK_RETURN: case SDLK_KP_ENTER:
        removerLocalConfirmado(); break;
      case SDLK_AC_BACK: case SDLK_ESCAPE: case SDLK_BACKSPACE: case SDLK_LEFT:
        confirmarRemocao[0] = 0; break;
      default: break;
    }
    return;
  }
  switch (e->key.keysym.sym) {
    case SDLK_UP:     if (foco > 0) foco--; else pediuRelogio = 1; break;
    case SDLK_DOWN:   if (foco < n + 1) foco++; break;
    case SDLK_RIGHT: {
      int idx = indiceAddon(foco, n);
      if (idx >= 0 && addonslocal_contem(addons_base(idx))) {
        snprintf(confirmarRemocao, sizeof confirmarRemocao, "%s", addons_base(idx));
      }
      break; }
    case SDLK_RETURN: case SDLK_KP_ENTER:
      if (foco == 0) { adicionarPorUrl(); break; }
      if (foco == 1) { sincronizar(); break; }
      {
        int idx = indiceAddon(foco, n);
        if (idx >= 0) {
          const char *url = addons_base(idx);
          if (addonslocal_contem(url) &&
              !addonslocal_ativar(url, !addons_ativo(idx))) {
            ilha_avisar("addon-local-erro", ILHA_INFO, "aj_puzzle",
                        "Não foi possível salvar a alteração.", 4000u, 0);
            break;
          }
          addons_alternar(idx);
          mexeu = 1;
        }
      }
      break;
    // Voltar (qualquer tecla de voltar) e Esquerda: de volta a linha "Addons"
    // em Ajustes › Fontes e addons (app.c chama ajustes_voltar_de_lista).
    case SDLK_AC_BACK: case SDLK_ESCAPE: case SDLK_BACKSPACE: case SDLK_LEFT:
      if (validandoUrl) ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle", "Cadastro cancelado.", 2500u, 0);
      addoninst_cancelar(); validandoUrl = aguardandoUrl = 0; sair = 1; break;
    default: break;
  }
}

// PONTEIRO (#99): a linha sob o cursor ganha o foco (a mesma variavel das
// setas). O desenho mora em ajustes_ux_ilha.inc (ajSubLista).
void addonsui_ponteiro(int i, int b) {
  (void)b;
  if (sair || confirmarRemocao[0] || i < 0 || i > addons_n() + 1 || i == foco) return;
  foco = i;
}
int addonsui_foco(void) { return foco; }

void addonsui_atualizar(float dt, Uint32 agora) {
  int i, n = addons_n();
  if (aguardandoUrl) {
    int resultado = teclado_resultado();
    if (resultado == TECLADO_PRONTO) {
      char url[NV_ADDON_URL_MAX];
      if (!nv_perfil_sessao_atual(sessaoCadastro)) {
        ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle",
                    "O perfil mudou. Abra novamente o cadastro do addon.", 4500u, 0);
      } else if (!addonslocal_normalizar(teclado_texto(), url, sizeof url)) {
        ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle",
                    "URL inválida. Copie o link completo do manifesto do addon.", 4500u, 0);
      } else if (addoninst_iniciar(url, sessaoCadastro)) {
        validandoUrl = 1;
        ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle",
                    "Verificando o manifesto do addon…", 12000u, 0);
      } else {
        ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle",
                    "Não foi possível iniciar a verificação. Tente novamente.", 4500u, 0);
      }
      aguardandoUrl = 0;
      foco = 0;
    } else if (resultado == TECLADO_CANCELOU) {
      aguardandoUrl = 0;
    }
  }
  if (validandoUrl) {
    AddonInstResultado r = {0};
    int estadoCadastro = addoninst_resultado(&r);
    if (estadoCadastro == 1) {
      validandoUrl = 0;
      if (r.erro[0]) {
        ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle", r.erro, 6000u, 0);
      } else if (!nv_perfil_sessao_travar(r.sessao)) {
        ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle",
                    "O perfil mudou. O addon não foi instalado.", 4500u, 0);
      } else {
        const char *url = r.url;
        char chave[32]; int duplicado = 0;
        nv_addon_chave(url, chave, sizeof chave);
        for (int k = 0; k < addons_n(); k++) {
          char existente[32]; nv_addon_chave(addons_base(k), existente, sizeof existente);
          if (!strcmp(chave, existente)) { duplicado = 1; break; }
        }
        if (duplicado) {
          ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle", "Addon já instalado neste perfil.", 4000u, 0);
        } else if (addons_n() >= ADDONS_MAX) {
          ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle", "Lista de addons cheia. Remova um antes de adicionar outro.", 4500u, 0);
        } else {
          int gravou = addonslocal_guardar(url);
          if (!gravou) {
            ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle", "Não foi possível salvar o addon nesta TV.", 4500u, 0);
          } else if (addons_adicionar(r.nome, url)) {
            mexeu = 1;
            addons_manifesto_recebido(url, addons_versao(), r.sessao, r.manifesto);
            ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle", "Addon verificado e adicionado a este perfil.", 4500u, 0);
          } else {
            if (gravou == 1) addonslocal_remover(url);
            ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle", "A lista mudou. Tente adicionar o addon novamente.", 4500u, 0);
          }
        }
        nv_perfil_sessao_destravar();
      }
      addoninst_liberar(&r);
    } else if (estadoCadastro < 0) {
      validandoUrl = 0;
      ilha_avisar("addon-cadastro", ILHA_INFO, "aj_puzzle", "Cadastro cancelado após mudança de perfil.", 4500u, 0);
    }
  }
  float alvo;
  (void)agora;
  for (i = 0; i < n && i < 128; i++)
    animFoco[i] = anim_mola(animFoco[i], i + 2 == foco ? 1.0f : 0.0f, dt, NV_MOLA_FOCO);
  // Rolagem que mantem a linha focada dentro da area util, como em Ajustes.
  alvo = (float)foco * (LINHA_H + LINHA_GAP);
  if (alvo - scrollY > BASE - TOPO - LINHA_H) scrollY = alvo - (BASE - TOPO - LINHA_H);
  if (alvo - scrollY < 0.0f) scrollY = alvo;
}

// "catalogo · fontes · legendas" com o que o manifesto respondeu. Enquanto a
// sonda nao voltou, dizer "nao fornece" seria afirmar o que nao se sabe.
static const char *capacidades(int i) {
  static char buf[96];
  // CADA PEDACO PASSA POR i18n(): a linha montada ("Catálogo  ·  Fontes")
  // nunca casa com chave nenhuma, e foi assim que a tela de addons ficou em
  // portugues no meio da interface em ingles (captura de 16/09).
  if (!addons_sondado(i)) return i18n("conferindo o manifesto…");
  buf[0] = 0;
  if (addons_fornece(i, ADD_CATALOGO)) strcat(buf, i18n("Catálogo"));
  if (addons_fornece(i, ADD_STREAM)) {
    if (buf[0]) strcat(buf, "  ·  ");
    strcat(buf, i18n("Fontes"));
  }
  if (addons_fornece(i, ADD_LEGENDA)) {
    if (buf[0]) strcat(buf, "  ·  ");
    strcat(buf, i18n("Legendas"));
  }
  if (!buf[0]) snprintf(buf, sizeof buf, "%s", i18n("não fornece nada que este app use"));
  return buf;
}

// A TELA DE ADDONS NO GLASS UI (mockup de Ajustes, quadro "addons"): o mesmo
// arranjo de ilhas de Ajustes, desenhado por ajustes.c (que tem as pecas).
void addonsui_desenhar(Uint32 agora) {
  (void)agora;
  ajustes_desenhar_addons(foco);
}

#ifdef AJUSTES_TESTE
void addonsui_teste_foco(int f) { foco = f; }
#endif
