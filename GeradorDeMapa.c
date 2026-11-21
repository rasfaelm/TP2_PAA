#include "GeradorDeMapa.h"
#include <stdlib.h>

#define CAMINHO    '0'
#define MONSTRO1   "-20"
#define MONSTRO2   "-10"
#define AVENTUREIRO 'I'
#define POCAO      "+20"
#define SAIDA      'F'

char **GeraMapa(int totalM1, int totalM2, int totalP, int linhas, int colunas) {
    if (linhas <= 0 || colunas <= 0 ||
        totalM1 < 0 || totalM2 < 0 || totalP < 0) {
        return NULL;
    }
    if ((totalM1 + totalM2 + totalP + 2) > linhas * colunas) {
        return NULL;
    }
    char **mapa = malloc(linhas * sizeof(char *));
    if (mapa == NULL) {
        return NULL;
    }
    for (int i = 0; i < linhas; i++) {
        mapa[i] = malloc(colunas * sizeof(char));
        if (mapa[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(mapa[j]);
            }
            free(mapa);
            return NULL;
        }
    }
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            mapa[i][j] = CAMINHO;
        }
    }
    mapa[linhas - 1][colunas - 1] = AVENTUREIRO;
    int linha, coluna;
    do {
        linha = rand() % linhas;
        coluna = rand() % colunas;
    } while (mapa[linha][coluna] != CAMINHO);
    mapa[linha][coluna] = SAIDA;
    for (int i = 0; i < totalM1; i++) {
        do {
            linha = rand() % linhas;
            coluna = rand() % colunas;
        } while (mapa[linha][coluna] != CAMINHO);
        mapa[linha][coluna] = MONSTRO1[1];
    }
    for (int i = 0; i < totalM2; i++) {
        do {
            linha = rand() % linhas;
            coluna = rand() % colunas;
        } while (mapa[linha][coluna] != CAMINHO);
        mapa[linha][coluna] = MONSTRO2[1];
    }
    for (int i = 0; i < totalP; i++) {
        do {
            linha = rand() % linhas;
            coluna = rand() % colunas;
        } while (mapa[linha][coluna] != CAMINHO);
        mapa[linha][coluna] = POCAO[1];
    }
    return mapa;
}
