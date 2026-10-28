#include "../src/ajustes.c"
#include <assert.h>
int selospacote_n(void){return 0;}
void ling_conta_legenda(const char *v){(void)v;}
void ling_conta_legenda2(const char *v){(void)v;}
void ling_conta_audio(const char *v){(void)v;}
void ling_conta_audio2(const char *v){(void)v;}
int main(void){
  assert(HOME_LAYOUT_MODERNA==0&&HOME_LAYOUT_PADRAO==1&&HOME_LAYOUT_DINAMICA==2);
  assert(HOME_LAYOUT_GLOBOPLAY==3&&HOME_LAYOUT_N==4);
  assert(valor[AJ_HOME_LAYOUT]==HOME_LAYOUT_MODERNA);
  assert(OPCOES[AJ_HOME_LAYOUT].n==HOME_LAYOUT_N);
  assert(!strcmp(V_HOME_LAYOUT[3],"Globoplay"));
  int topoSalvo=valor[AJ_MENU_TOPO];
  for(int i=0;i<HOME_LAYOUT_N;i++){
    valor[AJ_HOME_LAYOUT]=limita(AJ_HOME_LAYOUT,i);
    assert(ajustes_home_layout()==i);
    for(int t=0;t<2;t++){
      valor[AJ_MENU_TOPO]=t;
      assert(ajustes_menu_topo()==(i==3||(t==1&&i!=2)));
    }
  }
  valor[AJ_MENU_TOPO]=topoSalvo;
  valor[AJ_HOME_LAYOUT]=HOME_LAYOUT_GLOBOPLAY;
  assert(ajustes_menu_topo());
  assert(somenteDesteAparelho(AJ_HOME_LAYOUT));
  valor[AJ_HOME_LAYOUT]=HOME_LAYOUT_MODERNA;
  assert(valor[AJ_MENU_TOPO]==topoSalvo);
  valor[AJ_HOME_LAYOUT]=999;assert(ajustes_home_layout()==HOME_LAYOUT_MODERNA);
  puts("OK: quarto layout local; indices/padrao anteriores preservados, retorno e valores invalidos");
}
