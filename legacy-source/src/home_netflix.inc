/* Netflix is a presentation of the same catalog and playback requests.
 * No copied subscriptions, fabricated games, ranking or availability badges. */
static void netflixClique(int acao,int unused){
 (void)unused;if(!homeNetflixTV())return;
 netflixAcao=acao?1:0;focoHero=1;
 if(netflixAcao)pedidoAbrir=1;else pedidoTocar=1;
}
static int netflixTecla(SDL_Keycode k){
 if(!homeNetflixTV()||!focoHero)return 0;
 if(k==SDLK_LEFT){if(netflixAcao)netflixAcao=0;else pedidoMenu=1;return 1;}
 if(k==SDLK_RIGHT){if(!netflixAcao)netflixAcao=1;else {netflixAcao=0;heroPasso(1);}return 1;}
 return 0;
}
static void netflixBotao(GfxRect r,const char *label,int acao,int primary,float a){
 int f=focoHero&&netflixAcao==acao;
 float bg=primary?.96f:.25f;
 gfx_cor(r,.5f,bg,bg,bg,a*(primary?1:.94f));
 if(f)gfx_anel((GfxRect){r.x-4,r.y-4,r.w+8,r.h+8},.5f,2.6f,1,1,1,a);
 int ink=primary?18:248;
 TxtLinha l=txt_linha_corta(TXT_DET_BOTAO,label,ink,ink,ink,255,r.w-52);
 if(primary){gfx_icone((GfxRect){r.x+22,r.y+(r.h-24)*.5f,24,24},"play",.07f,.07f,.07f,a);}
 txt_desenhar_alpha(l,r.x+(r.w-l.w)*.5f+(primary?10:0),r.y+(r.h-l.h)*.5f,a);
#ifdef NV_TOUCH_UI
 if(a>.5f&&r.y>=132&&r.y+r.h<=NV_TELA_H)ponteiro_alvo(r.x,r.y,r.w,r.h,netflixClique,NULL,acao,0);
#endif
}
static void netflixPainel(GfxRect r,const CatItem *ci,const char *path,GLuint t,float video,float a){
 if(t&&home_proximo_desfocar(ci,path))t=gfx_desfocado(t,path);
 gfx_globoplay_media(r,t,path?tex_aspecto(path):16.0f/9.0f,video,a);
}
static void desenhaHeroNetflix(GfxRect r,const CatItem *ci,const CatItem *ant,
                              const char *arte,const char *arteAnt,GLuint t,GLuint tAnt,
                              float video,float a,float saida){
 (void)saida;if(a<=.004f)return;
 if(video>.004f)netflixPainel(r,ci,arte,t,video,a);
 else{
  if(tAnt&&heroSai>.004f)netflixPainel(r,ant,arteAnt,tAnt,0,a*anim_suave(heroSai));
  netflixPainel(r,ci,arte,t,0,a*(tAnt?anim_suave(heroEntra):1));
 }
 gfx_anel(r,18.0f/r.h,1.3f,.55f,.55f,.57f,a*.55f);
 if(!ci)return;
 SmNetflixHome n=sm_netflix_home(NV_TELA_W,NV_TELA_H);
 float x=r.x+38,base=r.y+r.h-62;
 float actionY=desenhaCopiaHero(ci,1,x,base,HOME_LAYOUT_NETFLIX,0,150,n.copyW,3,a,a,0,64,18,1);
 GfxRect play={x,actionY,220,64},info={x+238,actionY,310,64};
 netflixBotao(play,i18n(ci->progresso>0?"Retomar":"Assistir"),0,1,a);
 netflixBotao(info,i18n("Mais informações"),1,0,a);
 if(focoHero){
  itemFoco.indice=heroAtual;itemFoco.rect=r;itemFoco.arte=arteDoItem(ci,NULL);
  itemFoco.titulo=ci->titulo;itemFoco.genero=ci->genero;itemFoco.meta=ci->meta;publicarItemFoco();
 }
}
