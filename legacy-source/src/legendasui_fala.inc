// Painel ancorado a ilha existente. Nada baixa ou analisa audio aqui.
// O documento e retido uma vez ao abrir; somente as linhas visiveis desenham.
static LegFala fala;
static float falaScroll, falaScrollMax;
static int falaEsperaSoltar;
static unsigned falaTela;
static char falaErro[180];

int legendasui_fala_aberta(void) { return falaAberta; }
void legendasui_fala_fechar(void) {
  legfala_fechar(&fala); falaAberta = 0; falaScroll = falaScrollMax = 0;
  falaEsperaSoltar = 0; falaErro[0] = 0; falaTela++;
}
static int falaVigente(void) {
  return falaAberta && legsync_fala_valida(fala.doc, fala.geracao, video_url_atual());
}
static void falaAbrir(void) {
  unsigned geracao = 0;
  LegendaDocumento *doc = legsync_fala_documento(&geracao);
  legendasui_fala_fechar();
  if (!doc || player_id_canal()[0] ||
      !legsync_fala_valida(doc, geracao, video_url_atual()) || !legfala_abrir(&fala, doc, geracao)) {
    legenda_documento_liberar(doc);
    avisar(i18n("Escolha uma legenda de texto de um addon e aguarde o carregamento."));
    return;
  }
  legenda_documento_liberar(doc);
  falaAberta = 1; falaEsperaSoltar = 1; menu = okPendente = okLongo = 0;
}
static void falaAtualizar(void) {
  if (falaAberta && !falaVigente()) {
    legendasui_fala_fechar();
    avisar(i18n("A legenda ou a fonte mudou. Marque a fala novamente."));
  }
}
static void falaMarcar(void) {
  if (!falaVigente()) { falaAtualizar(); return; }
  double pos = player_posicao_seg();
  double texto = legsync_posicao(pos) * 1000 + legsync_offset_ms(player_leg_estilo()->atrasoMs);
  if (!legfala_capturar(&fala, pos * 1000, texto)) {
    snprintf(falaErro, sizeof falaErro, "%s", i18n("Aguarde o início da reprodução e tente novamente.")); return;
  }
  falaScroll = falaScrollMax = 0; falaErro[0] = 0; falaTela++;
}
static void falaAplicar(void) {
  int atraso;
  if (!falaVigente()) { falaAtualizar(); return; }
  if (!legfala_calcular(&fala, &atraso)) {
    snprintf(falaErro, sizeof falaErro, "%s", i18n("Ajuste maior que 60 segundos. Marque outra fala ou escolha uma frase mais próxima."));
    return;
  }
  if (legsync_fala_aplicar(fala.doc, fala.geracao, video_url_atual(), player_leg_estilo()->atrasoMs, atraso)) {
    legendasui_fala_fechar();
    avisar(i18n("Legenda alinhada pela fala. Use Atraso para ajustar mais fino."));
  } else falaAtualizar();
}
static int falaEvento(const SDL_Event *e) {
  if (e->type != SDL_KEYDOWN && e->type != SDL_KEYUP) return LEGUI_TRATADO;
  SDL_Keycode k = e->key.keysym.sym;
  int ok = k == SDLK_RETURN || k == SDLK_KP_ENTER || k == SDLK_SPACE;
  if (e->type == SDL_KEYUP) { if (ok) falaEsperaSoltar = 0; return LEGUI_TRATADO; }
  if (!falaVigente()) { falaAtualizar(); return LEGUI_TRATADO; }
  if (k == SDLK_AC_BACK || k == SDLK_ESCAPE || k == SDLK_BACKSPACE) {
    if (e->key.repeat) return LEGUI_TRATADO;
    if (fala.capturada) { fala.capturada = 0; falaErro[0] = 0; falaScroll = 0; falaTela++; }
    else legendasui_fala_fechar();
  } else if (ok && !e->key.repeat && !falaEsperaSoltar) {
    falaEsperaSoltar = 1;
    if (fala.capturada) falaAplicar(); else falaMarcar();
  } else if (fala.capturada) {
    if ((k == SDLK_UP || k == SDLK_DOWN) && legfala_mover(&fala, k == SDLK_UP ? -1 : 1)) {
      falaScroll = 0; falaErro[0] = 0;
    } else if (k == SDLK_LEFT || k == SDLK_RIGHT)
      falaScroll = fminf(falaScrollMax, fmaxf(0, falaScroll + (k == SDLK_RIGHT ? 60 : -60)));
  }
  return LEGUI_TRATADO;
}
static int falaPonteiroValido(int token) { return falaVigente() && (unsigned)token == falaTela; }
static void falaPonteiroFoco(int i, int token) {
  if (!falaPonteiroValido(token) || !fala.capturada || i < 0 || i >= fala.n) return;
  if (fala.foco != i) { fala.foco = i; falaScroll = 0; falaErro[0] = 0; }
}
static void falaPonteiroAplicar(int i, int token) {
  if (!falaPonteiroValido(token) || !fala.capturada || i < 0 || i >= fala.n) return;
  falaPonteiroFoco(i, token); falaAplicar();
}
static void falaPonteiroMarcar(int i, int token) {
  (void)i;
  if (falaPonteiroValido(token)) { falaMarcar(); falaEsperaSoltar = 0; }
}
static void falaTexto(TxtEstilo e, const char *s, float x, float y, float w, float a, int tom) {
  TxtLinha t = txt_linha_corta(e, s, tom, tom, tom, 255, w);
  txt_desenhar_alpha(t, x, y, a);
}
static void falaTempo(double segundos, char *dst, size_t tam) {
  if (!isfinite(segundos) || segundos < 0 || segundos > 35999999) {
    snprintf(dst,tam,"--:--:--"); return;
  }
  int t = (int)segundos;
  snprintf(dst,tam,"%02d:%02d:%02d",t/3600,(t/60)%60,t%60);
}
static void falaDesenhar(GfxRect c, float a) {
  float x = c.x + 28, y = c.y + 22, w = c.w - 56;
  falaTexto(TXT_ILHA_PERGUNTA, "Sincronizar pela fala", x, y, w, a, 245);
  y += 58;
  // Durante a expansao da ilha o corpo ainda pode ter a altura da pagina
  // anterior. Aguarda espaco para nao criar alvos invisiveis fora do painel.
  if (c.h < 540) return;
  if (!fala.capturada) {
    txt_bloco(TXT_ILHA_CORPO, "Quando ouvir o início de uma fala, pressione OK. Depois escolha a frase correspondente na legenda.",
        210, 218, 232, x, y + 16, w, 34, a, 0);
    GfxRect b = { x, c.y + c.h * .49f, w, 82 };
    plrui_linha_foco(b, 18, a);
    falaTexto(TXT_ILHA_FORTE, "Marcar fala agora", b.x + 22, b.y + 25, b.w - 44, a, 250);
    if (a > .99f) ponteiro_alvo(b.x,b.y,b.w,b.h,NULL,falaPonteiroMarcar,0,(int)falaTela);
    if (falaErro[0]) txt_bloco(TXT_ILHA_APOIO, falaErro, 247, 162, 147, x, b.y + 102, w, 27, a, 0);
    falaTexto(TXT_ILHA_APOIO, "OK: marcar fala    Voltar: cancelar", x, c.y + c.h - 48, w, a, 180);
    return;
  }
  char rotulo[100], marca[32];
  falaTempo(fala.capturaMs/1000,marca,sizeof marca);
  snprintf(rotulo,sizeof rotulo,"Fala marcada em %s  ·  Frase %d de %d",marca,fala.foco+1,fala.n);
  falaTexto(TXT_ILHA_APOIO,rotulo,x,y,w,a,190); y += 38;
  int vis = c.h < 720 ? 3 : 4;
  int ini = fala.foco - vis / 2;
  if (ini + vis > fala.n) ini = fala.n - vis;
  if (ini < 0) ini = 0;
  for (int j = 0; j < vis && ini + j < fala.n; j++) {
    int i = ini + j;
    const LegendaCue *cue = legfala_cue(&fala, i);
    char texto[sizeof cue->texto], tempo[32];
    legfala_frase(cue,texto,sizeof texto);
    for (char *p = texto; *p; p++) if (*p == '\n' || *p == '\r') *p = ' ';
    falaTempo(cue->inicio,tempo,sizeof tempo);
    GfxRect b = {x,y,w,58};
    if (i == fala.foco) plrui_linha_foco(b,14,a);
    else gfx_cor(b,14 / b.h,.08f,.11f,.16f,.6f*a);
    falaTexto(TXT_ILHA_APOIO,tempo,x+14,y+18,116,a,i==fala.foco?245:170);
    falaTexto(TXT_ILHA_CORPO,texto,x+146,y+16,w-164,a,i==fala.foco?250:210);
    if (a > .99f) ponteiro_alvo(b.x,b.y,b.w,b.h,falaPonteiroFoco,falaPonteiroAplicar,i,(int)falaTela);
    y += 64;
  }
  y += 12;
  GfxRect detalhe = {x,y,w,fmaxf(30,c.y+c.h-116-y)};
  gfx_cor(detalhe,16/detalhe.h,.035f,.055f,.095f,.95f*a);
  char frase[768]; legfala_frase(legfala_cue(&fala,fala.foco),frase,sizeof frase);
  float altura = txt_bloco(TXT_ILHA_CORPO,frase,240,244,250,x+18,0,w-36,34,0,0);
  falaScrollMax = fmaxf(0,altura - detalhe.h + 28);
  falaScroll = fminf(falaScroll,falaScrollMax);
  gfx_recorte(detalhe.x,detalhe.y,detalhe.w,detalhe.h);
  // Sem reticencias: a frase inteira continua acessivel por esquerda/direita.
  txt_bloco(TXT_ILHA_CORPO,frase,240,244,250,x+18,y+14-falaScroll,w-36,34,a,0);
  gfx_sem_recorte();
  if (falaErro[0]) txt_bloco_corta(TXT_ILHA_APOIO,falaErro,247,162,147,x,c.y+c.h-102,w,27,a,2);
  else falaTexto(TXT_ILHA_APOIO,"OK: aplicar    ↑↓: escolher frase    ←→: ler trecho completo",x,c.y+c.h-88,w,a,195);
  falaTexto(TXT_ILHA_APOIO,"Voltar: marcar outra fala ou cancelar",x,c.y+c.h-48,w,a,180);
}
