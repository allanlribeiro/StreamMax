#ifndef STREAMMAX_ADDONSLOCAL_H
#define STREAMMAX_ADDONSLOCAL_H
#include "addons.h"

/* Instalações locais, privadas ao par conta/perfil. NÃO são enviadas ao servidor.
 * Retornos: 1 = novo cadastro, 2 = já cadastrado, 0 = URL/armazenamento inválido. */
int addonslocal_guardar(const char *url);

/* Mescla os addons pessoais sem sobrescrever os da conta; deduplica por URL
 * normalizada. A saída deve comportar no máximo ADDONS_MAX elementos. */
int addonslocal_combinar(const AddonRemoto *conta, int n,
                         AddonRemoto *saida, int capacidade);
#endif
