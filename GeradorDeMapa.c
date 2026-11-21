#include "GeradorDeMapa.h"
#include <stdlib.h>
#include <string.h>

#define CAMINHO     "0"
#define MONSTRO1    "-20"
#define MONSTRO2    "-10"
#define AVENTUREIRO "I"
#define POCAO       "+20"
#define SAIDA       "F"

static void colocar(char ***mapa, int linhas, int colunas, const char *tipo) {
    int l, c;
    do {
        l = rand() % linhas;
        c = rand() % colunas;
    } while (strcmp(mapa[l][c], CAMINHO) != 0);
    mapa[l][c] = (char *)tipo;
}

char ***GeraMapa(int totalM1, int totalM2, int totalP, int linhas, int colunas) {
    if (linhas <= 0 || colunas <= 0 ||
        totalM1 < 0 || totalM2 < 0 || totalP < 0) {
        return NULL;
    }
    if ((long long)totalM1 + totalM2 + totalP + 2 > (long long)linhas * colunas) {
        return NULL;
    }

    char ***mapa = malloc(linhas * sizeof(char **));
    if (mapa == NULL) {
        return NULL;
    }
    for (int i = 0; i < linhas; i++) {
        mapa[i] = malloc(colunas * sizeof(char *));
        if (mapa[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(mapa[j]);
            }
            free(mapa);
            return NULL;
        }
        for (int j = 0; j < colunas; j++) {
            mapa[i][j] = CAMINHO;
        }
    }
    mapa[linhas - 1][colunas - 1] = AVENTUREIRO;
    colocar(mapa, linhas, colunas, SAIDA);
    for (int i = 0; i < totalM1; i++) colocar(mapa, linhas, colunas, MONSTRO1);
    for (int i = 0; i < totalM2; i++) colocar(mapa, linhas, colunas, MONSTRO2);
    for (int i = 0; i < totalP;  i++) colocar(mapa, linhas, colunas, POCAO);
    return mapa;
}

void LiberaMapa(char ***mapa, int linhas, int colunas) {
    (void)colunas;
    if (mapa == NULL) return;
    for (int i = 0; i < linhas; i++) {
        free(mapa[i]);
    }
    free(mapa);
}

int MapaParaCaverna(char ***mapa, int linhas, int colunas, long long vidaInicial, Caverna *c) {
    c->linhas = linhas;
    c->colunas = colunas;
    c->vidaInicial = vidaInicial;
    c->li = c->ci = c->lf = c->cf = -1;
    c->valor = (long long *)calloc((size_t)linhas * colunas, sizeof(long long));
    if (!c->valor) {
        fprintf(stderr, "Memoria insuficiente.\n");
        return -1;
    }
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            const char *t = mapa[i][j];
            if (strcmp(t, AVENTUREIRO) == 0) {
                c->li = i; c->ci = j;
            } else if (strcmp(t, SAIDA) == 0) {
                c->lf = i; c->cf = j;
            } else {
                c->valor[IDX(c, i, j)] = strtoll(t, NULL, 10);
            }
        }
    }
    if (c->li < 0 || c->lf < 0) {
        fprintf(stderr, "Mapa sem 'I' ou sem 'F'.\n");
        liberarCaverna(c);
        return -1;
    }
    return 0;
}