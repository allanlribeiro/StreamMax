#ifndef STREAMMAX_ADDONSLOCAL_H
#define STREAMMAX_ADDONSLOCAL_H
#include "addons.h"

/* URLs locais privadas por conta/perfil. Nunca enviar ao backend.
 * 1 = gravou, 2 = já cadastrado, 0 = erro/URL inválida. */
int addonslocal_guardar(const char *url);

/* 1 quando o URL foi cadastrado por esta TV; 0 caso contrário. */
int addonslocal_contem(const char *url);

/* Persistir estado ligado/desligado; 1 = sucesso, 0 = não cadastrado/erro. */
int addonslocal_ativar(const char *url, int ativo);

/* Remover da lista local; 1 = sucesso, 0 = não cadastrado/erro. */
int addonslocal_remover(const char *url);

/* Mescla lista do servidor com os locais (sem duplicatas).
 * Preserva os estados dos locais; os do servidor têm prioridade. */
int addonslocal_combinar(const AddonRemoto *conta, int n,
                         AddonRemoto *saida, int capacidade);
#endif
