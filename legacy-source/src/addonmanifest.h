#ifndef STREAMMAX_ADDONMANIFEST_H
#define STREAMMAX_ADDONMANIFEST_H
#include <stddef.h>
#define ADDON_MANIFESTO_MAX (2u * 1024u * 1024u)
typedef enum { AM_OK, AM_INVALIDO, AM_CONFIGURAR } AddonManifestoEstado;
/* Sem rede, logs ou armazenamento. Exige documento JSON completo e campos da raiz. */
AddonManifestoEstado addonmanifest_validar(const char *json, size_t n, char *nome, size_t tamanho);
#endif
