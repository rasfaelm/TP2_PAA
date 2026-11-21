#ifndef ARTE_H
#define ARTE_H

#include "caverna.h"

void menu(void);

void printarMapa(char ***mapa, int linhas, int colunas);
void printarCaverna(const Caverna *c, const char *visitado, int pl, int pc);

#endif