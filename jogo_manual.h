#ifndef JOGO_MANUAL_H
#define JOGO_MANUAL_H

#include "caverna.h"

/* Modo interativo: o usuario move o personagem pela caverna digitando
   comandos no teclado (W/A ou setas, z = desfazer, r = reiniciar,
   q = sair). Ao chegar em F com vida, grava o caminho percorrido em `saida`
   (mesmo formato de resultado.txt) e compara com o otimo da PD.
   Retorna 0 se terminou normalmente, -1 em erro. */
int jogarCaverna(const Caverna *c, const char *saida);

#endif