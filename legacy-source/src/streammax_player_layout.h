#ifndef STREAMMAX_PLAYER_LAYOUT_H
#define STREAMMAX_PLAYER_LAYOUT_H

/* Geometria pura da faixa de controles, equivalente ao comportamento dos
 * IconButton da interface Android original, sem dependencias de SDL/GL.
 * O desenho usa a paleta StreamMax, o motor de video permanece inalterado. */
static inline float streammax_player_controle_diametro(float largura,
                                                       int quantidade,
                                                       float diametro,
                                                       float intervalo) {
  float cabe;
  if (quantidade < 1 || largura <= 0.0f || diametro <= 0.0f) return 0.0f;
  if (quantidade == 1) return largura < diametro ? largura : diametro;
  cabe = (largura - (float)(quantidade - 1) * intervalo) / (float)quantidade;
  if (cabe <= 0.0f) return 0.0f;
  return cabe < diametro ? cabe : diametro;
}

static inline float streammax_player_controle_x(float esquerda,
                                                int indice,
                                                float diametro,
                                                float intervalo) {
  return esquerda + (float)indice * (diametro + intervalo);
}

static inline float streammax_player_foco_opacidade(float foco) {
  return foco <= 0.0f ? 0.0f : foco >= 1.0f ? 1.0f : foco;
}

#endif
