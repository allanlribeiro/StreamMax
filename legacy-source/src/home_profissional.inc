/* A opcao Profissional compartilha catalogo, colecoes, foco e player.
   Apenas o layout selecionado percorre estes atalhos; nenhuma consulta de
   streams/TMDB e disparada pelo desenho. As marcas vem dos dados reais. */
static int profissionalHubs(void) { return nProfPastas+1; }
static void profissionalEscolher(int qual) {
  if(qual<0||qual>=profissionalHubs())return;
  if(qual<nProfPastas){
    int id=profPastas[qual];
    if(id>=0){const ColFolder *f=col_folder(id);if(f)vertudo_colecao(f);}
    else{
      int r=-id-1;
      if(r>=0&&r<nFileiras&&fileiras[r].base[0])
        vertudo_abrir(fileiras[r].base,fileiras[r].catTipo,fileiras[r].catId,fileiras[r].titulo);
    }
  }else{pedidoGuia=1;guiaId[0]=0;}
}
static void profissionalClique(int qual,int extra) {
  (void)extra;
  if(layoutHome()!=HOME_LAYOUT_PROFISSIONAL)return;
  profHubFoco=qual;focoHero=1;profissionalEscolher(qual);
}
/* O trilho de colecoes e um degrau entre o Hero e as fileiras existentes.
   Nao ocupa indices do catalogo nem passa um atalho como se fosse um filme. */
static int profissionalTecla(SDL_Keycode k) {
  if(layoutHome()!=HOME_LAYOUT_PROFISSIONAL||!ajustes_hero_ligado()||!focoHero)return 0;
  int n=profissionalHubs();
  if(profHubFoco>=n)profHubFoco=n-1;
  if(profHubFoco<0){
    if(k==SDLK_DOWN){profHubFoco=0;return 1;}
    return 0;
  }
  if(k==SDLK_RIGHT){if(profHubFoco+1<n)profHubFoco++;return 1;}
  if(k==SDLK_LEFT){if(profHubFoco>0)profHubFoco--;else pedidoMenu=1;return 1;}
  if(k==SDLK_UP){profHubFoco=-1;return 1;}
  if(k==SDLK_DOWN){
    int primeira=primeiraFileiraNavegavel();
    if(primeira>=0){
      foco.fileira=primeira;foco.coluna=foco.colunaLembrada[primeira];
      if(foco.coluna<0||foco.coluna>=foco.nColunas[primeira])foco.coluna=0;
      focoHero=0;
    }
    return 1;
  }
  return 0;
}
static void profissionalColecoes(void) {
  nProfPastas=0;
  if(layoutHome()!=HOME_LAYOUT_PROFISSIONAL){profHubFoco=-1;return;}
  /* So fileiras visiveis; prioriza Streaming e preserva a ordem declarada.
     Nao inclui grupos desativados nem fabrica marcas para contas vazias. */
  for(int fase=0;fase<2&&nProfPastas<SM_PROF_PASTAS_MAX;fase++)
    for(int r=0;r<nFileiras&&nProfPastas<SM_PROF_PASTAS_MAX;r++){
      const Fileira *row=&fileiras[r];
      if(row->tipo!=FILEIRA_CATALOGOS)continue;
      if((strcasecmp(row->titulo,"Streaming")==0)!=(fase==0))continue;
      for(int c=0;c<row->n&&nProfPastas<SM_PROF_PASTAS_MAX;c++){
        int id=row->folders[c],repetido=0;
        const ColFolder *f=col_folder(id);
        if(!f||!f->nSources)continue;
        for(int j=0;j<nProfPastas;j++)if(profPastas[j]==id)repetido=1;
        if(!repetido)profPastas[nProfPastas++]=id;
      }
    }
  /* Sem colecoes da conta, atalhos dos catalogos que JA existem. Nao criar
     assinaturas ficticias nem deixar toda a faixa vazia para contas novas. */
  if(!nProfPastas)
    for(int r=0;r<nFileiras&&nProfPastas<SM_PROF_PASTAS_MAX;r++)
      if(fileiras[r].base[0]&&fileiras[r].catId[0]&&
         (!strcmp(fileiras[r].catTipo,"movie")||!strcmp(fileiras[r].catTipo,"series")))
        profPastas[nProfPastas++]=-r-1;
  if(profHubFoco>=profissionalHubs())profHubFoco=profissionalHubs()-1;
}
static GfxRect profissionalHubRect(int i,float offset) {
  SmProfHome p=sm_prof_home(NV_TELA_W,NV_TELA_H);
  int n=profissionalHubs();
  float gap=18.0f,w=fminf(360.0f,(NV_TELA_W-2*p.margin-gap*(n-1))/n);
  return (GfxRect){p.margin+i*(w+gap),p.hubY-offset,w,p.hubH};
}
static void profissionalHubsDesenhar(float offset,float alfa) {
  if(alfa<=.004f)return;
  int n=profissionalHubs();
  for(int i=0;i<n;i++){
    GfxRect r=profissionalHubRect(i,offset);
    if(r.y+r.h<132||r.y>NV_TELA_H)continue;
    int f=focoHero&&profHubFoco==i;
    gfx_cor(r,14.0f/r.h,f?.12f:.065f,f?.15f:.075f,f?.20f:.095f,alfa);
    if(f)gfx_anel(r,14.0f/r.h,2.5f,.88f,.94f,1,alfa);
    if(i<nProfPastas){
      int id=profPastas[i],row=-id-1;
      const ColFolder *folder=id>=0?col_folder(id):NULL;
      const char *title=folder?folder->title:row>=0&&row<nFileiras?fileiras[row].titulo:"";
      const char *logo=folder&&folder->logo[0]?folder->logo:NULL;
      GLuint t=logo?tex_obter_larg(logo,256):0;
      float aspect=logo?tex_aspecto(logo):0;
      if(t&&aspect>0){
        float w=fminf(r.w-48,aspect*(r.h-38)),h=w/aspect;
        gfx_rect((GfxRect){r.x+(r.w-w)*.5f,r.y+(r.h-h)*.5f,w,h},t,GFX_TEXTO,0,0,0,0,1,1,1,alfa);
      }else if(title[0]){
        uint64_t mask=badges_provedor(title);
        if(mask)badges_desenhar(mask,r.x+24,r.y+(r.h-36)*.5f,r.w-48,36,alfa);
        else{
          TxtLinha l=txt_linha_corta(TXT_DET_BOTAO,title,240,243,248,255,r.w-36);
          txt_desenhar_alpha(l,r.x+(r.w-l.w)*.5f,r.y+(r.h-l.h)*.5f,alfa);
        }
      }
    }else{
      gfx_icone((GfxRect){r.x+22,r.y+(r.h-34)*.5f,34,34},"aj_tv-minimal-play",.88f,.94f,1,alfa);
      TxtLinha l=txt_linha_corta(TXT_DET_BOTAO,i18n("TV ao vivo"),245,247,250,255,r.w-92);
      txt_desenhar_alpha(l,r.x+74,r.y+19,alfa);
      l=txt_linha_corta(TXT_CAPTION2,i18n("IPTV e VOD"),175,186,202,255,r.w-92);
      txt_desenhar_alpha(l,r.x+74,r.y+r.h-36,alfa);
    }
#ifdef NV_TOUCH_UI
    if(alfa>.25f&&r.y>=132&&r.y+r.h<=NV_TELA_H)
      ponteiro_alvo(r.x,r.y,r.w,r.h,profissionalClique,NULL,i,0);
#endif
  }
}
static void profissionalFontes(const CatItem *ci,float x,float y,float a) {
  if(!ci||!ci->provNome[0]||a<=.004f)return;
  uint64_t mask=badges_provedor(ci->provNome);
  if(!mask)return;
  TxtLinha l=txt_linha(TXT_CAPTION2,i18n("Serviços do catálogo"),184,194,208,255);
  txt_desenhar_alpha(l,x,y,a);
  /* Assinatura informada pelos metadados; aluguel/compra e fontes dos
     addons continuam separados na pagina do titulo. Nao inferir do trailer. */
  badges_desenhar(mask,x+l.w+20,y,300,25,a);
}
static void profissionalPainel(GfxRect r,const CatItem *ci,const char *path,GLuint t,float video,float a) {
  if(t&&home_proximo_desfocar(ci,path))t=gfx_desfocado(t,path);
  gfx_profissional_media(r,t,path?tex_aspecto(path):16.0f/9.0f,video,a);
}
static void desenhaHeroProfissional(GfxRect r,const CatItem *ci,const CatItem *ant,
                                   const char *arte,const char *arteAnt,GLuint t,GLuint tAnt,
                                   float video,float alfa,float saida) {
  if(alfa<=.004f)return;
  SmProfHome p=sm_prof_home(NV_TELA_W,NV_TELA_H);
  float offset=scrollY+empurraHero();
  if(video>.0f)profissionalPainel(r,ci,arte,t,video,alfa);
  else{
    if(tAnt&&heroSai>.004f)profissionalPainel(r,ant,arteAnt,tAnt,0,alfa*anim_suave(heroSai));
    profissionalPainel(r,ci,arte,t,0,alfa*(tAnt?anim_suave(heroEntra):1));
  }
  if(!ci)return;
  float base=p.base-offset;
  float actionY=desenhaCopiaHero(ci,1,p.margin,base,HOME_LAYOUT_PROFISSIONAL,0,112,
                                p.copyW,2,alfa,alfa,0,54,16,1);
  GfxRect bt={p.margin,actionY,230,54};
  int focus=focoHero&&profHubFoco<0;
  gfx_cor(bt,.5f,focus?.94f:.14f,focus?.96f:.17f,focus?1:.22f,alfa);
  if(focus)gfx_anel((GfxRect){bt.x-4,bt.y-4,bt.w+8,bt.h+8},.5f,2,.85f,.92f,1,alfa*.6f);
  TxtLinha l=txt_linha(TXT_DET_BOTAO,i18n("Ver título"),focus?18:245,focus?23:245,focus?30:245,255);
  txt_desenhar_alpha(l,bt.x+(bt.w-l.w)*.5f,bt.y+(bt.h-l.h)*.5f,alfa);
  profissionalFontes(ci,p.margin,p.heroH-42-offset,alfa);
#ifdef NV_TOUCH_UI
  if(focus){alvoAcaoHero(bt,saida,0,alfa);heroPublicarFoco(ci,alfa);}
#endif
  if(focus&&!telefoneui_ativo()){
    itemFoco.indice=heroAtual;itemFoco.rect=r;itemFoco.arte=arteDoItem(ci,NULL);
    itemFoco.titulo=ci->titulo;itemFoco.genero=ci->genero;itemFoco.meta=ci->meta;publicarItemFoco();
  }
}
