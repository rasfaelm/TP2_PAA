#ifndef GERADORDEMAPA_H
#define GERADORDEMAPA_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "caverna.h"

/* Gera um mapa aleatorio. Cada celula e uma string: "0" (caminho), "-20" e "-10" (monstros),
   "+20" (pocao), "I" (inicio, sempre no canto inferior direito) e "F" (saida).
   Retorna NULL se os parametros forem invalidos ou nao couberem no mapa.
   Lembre-se de chamar srand() antes. */
char ***GeraMapa(int totalM1, int totalM2, int totalP, int linhas, int colunas);

/* Libera um mapa criado por GeraMapa. */
void LiberaMapa(char ***mapa, int linhas, int colunas);

/* Converte o mapa gerado para uma Caverna (a mesma estrutura usada pela PD,
   pelo guloso e por salvarCaverna). Retorna 0 em sucesso, -1 em erro. */
int MapaParaCaverna(char ***mapa, int linhas, int colunas, long long vidaInicial, Caverna *c);

#endif