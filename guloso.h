#ifndef GULOSO_H
#define GULOSO_H

#include "caverna.h"

/* Resolve a caverna com a heuristica gulosa (a cada passo vai para a vizinha,
   entre cima e esquerda, de maior valor), grava o caminho em `saida`
   e imprime uma comparacao com a solucao otima da programacao dinamica.
   Retorna 0 em sucesso, -1 em erro. */
int compararGulosoComPD(const Caverna *c, const char *saida);

/* Versao sem arquivo/saida (usada no bench): retorna 1 se o guloso chegou em F,
   0 se morreu no caminho, -1 se F nao esta acima/a esquerda de I.
   *pvFinal recebe os PV ao final (ou ao morrer). */
int resolverGuloso(const Caverna *c, long long *pvFinal);

#endif