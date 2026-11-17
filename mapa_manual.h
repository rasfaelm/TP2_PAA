#ifndef MAPA_MANUAL_H
#define MAPA_MANUAL_H

#include "caverna.h"

/* Cria uma caverna interativamente (stdin): pergunta linhas, colunas, PV
   iniciais e depois cada linha da matriz. Aceita inteiros, 'I' (inicio) ou 'F' (fim).
   Linhas invalidas sao pedidas de novo. */
int criarCavernaManual(Caverna *c);

/* Grava a caverna no mesmo formato lido por lerCaverna().*/
int salvarCaverna(const Caverna *c, const char *caminho);

#endif