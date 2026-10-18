#include "../src/streammax_player_layout.h"
#include <assert.h>

static int proximo(float a, float b) {
  return a - b < 0.001f && b - a < 0.001f;
}

int main(void) {
  /* TV grande: botoes redondos fixos, sem pílula expansivel. */
  assert(proximo(streammax_player_controle_diametro(1700.0f, 8, 68.0f, 12.0f), 68.0f));
  assert(proximo(streammax_player_controle_x(96.0f, 2, 68.0f, 12.0f), 256.0f));
  /* TV estreita: diminuicao uniforme, foco nao desloca os vizinhos. */
  assert(proximo(streammax_player_controle_diametro(320.0f, 5, 68.0f, 12.0f), 54.4f));
  assert(streammax_player_controle_diametro(10.0f, 8, 68.0f, 12.0f) == 0.0f);
  assert(streammax_player_controle_diametro(100.0f, 0, 68.0f, 12.0f) == 0.0f);
  assert(streammax_player_foco_opacidade(-1.0f) == 0.0f);
  assert(streammax_player_foco_opacidade(2.0f) == 1.0f);
  assert(proximo(streammax_player_foco_opacidade(0.35f), 0.35f));
  return 0;
}
