#include "../src/ajustes.c"
#include <assert.h>
static int mobileFixture;
int layout_modo_mobile(void){return mobileFixture;}
int selospacote_n(void){return 0;}
void ling_conta_legenda(const char *v){(void)v;}
void ling_conta_legenda2(const char *v){(void)v;}
void ling_conta_audio(const char *v){(void)v;}
void ling_conta_audio2(const char *v){(void)v;}
int main(void){
  assert(HOME_LAYOUT_MODERNA==0&&HOME_LAYOUT_PADRAO==1&&HOME_LAYOUT_DINAMICA==2);
  assert(HOME_LAYOUT_GLOBOPLAY==3&&HOME_LAYOUT_PROFISSIONAL==4&&HOME_LAYOUT_NETFLIX==5&&HOME_LAYOUT_N==6);
  assert(valor[AJ_HOME_LAYOUT]==HOME_LAYOUT_MODERNA);
  assert(OPCOES[AJ_HOME_LAYOUT].n==HOME_LAYOUT_N);
  assert(!strcmp(V_HOME_LAYOUT[3],"Globoplay"));
  assert(!strcmp(V_HOME_LAYOUT[4],"Profissional"));
  assert(!strcmp(V_HOME_LAYOUT[5],"Netflix"));
  int topoSalvo=valor[AJ_MENU_TOPO];
  for(int i=0;i<HOME_LAYOUT_N;i++){
    valor[AJ_HOME_LAYOUT]=limita(AJ_HOME_LAYOUT,i);
    assert(ajustes_home_layout()==i);
    for(int t=0;t<2;t++){
      valor[AJ_MENU_TOPO]=t;
      assert(ajustes_menu_topo()==(i==3||i==4||i==5||(t==1&&i!=2)));
    }
  }
  mobileFixture=1;valor[AJ_HOME_LAYOUT]=HOME_LAYOUT_NETFLIX;valor[AJ_MENU_TOPO]=0;
  assert(!ajustes_menu_topo());mobileFixture=0;
  valor[AJ_MENU_TOPO]=topoSalvo;
  valor[AJ_HOME_LAYOUT]=HOME_LAYOUT_GLOBOPLAY;
  assert(ajustes_menu_topo());
  assert(somenteDesteAparelho(AJ_HOME_LAYOUT));
  valor[AJ_HOME_LAYOUT]=HOME_LAYOUT_MODERNA;
  assert(valor[AJ_MENU_TOPO]==topoSalvo);
  valor[AJ_HOME_LAYOUT]=999;assert(ajustes_home_layout()==HOME_LAYOUT_MODERNA);
  assert(OPCOES[AJ_PS_FUNDO].n==6 && !strcmp(V_PS_FUNDO[5],"StreamMax"));
  for(int i=0;i<6;i++) { valor[AJ_PS_FUNDO]=limita(AJ_PS_FUNDO,i); assert(ajustes_ps_fundo()==i); }
  valor[AJ_PS_FUNDO]=999; assert(ajustes_ps_fundo()==5);
  puts("OK: seis layouts locais; indices/padrao anteriores preservados, retorno e valores invalidos");
}
