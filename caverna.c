#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "caverna.h"

#define INVALIDO LLONG_MIN


#define IDX(cav, i, j) ((size_t)(i) * (cav)->colunas + (j))

/* Lê o arquivo de entrada. Retorna 0 em caso de sucesso. */
int lerCaverna(const char *caminho, Caverna *c) {
    FILE *f = fopen(caminho, "r");
    if (!f) {
        perror("Erro ao abrir arquivo de entrada");
        return -1;
    }
    if (fscanf(f, "%d %d %lld", &c->linhas, &c->colunas, &c->vidaInicial) != 3) {
        fprintf(stderr, "Cabeçalho inválido.\n");
        fclose(f);
        return -1;
    }

    size_t total = (size_t)c->linhas * c->colunas;
    c->valor = (long long *)calloc(total, sizeof(long long));
    if (!c->valor) {
        fprintf(stderr, "Memória insuficiente.\n");
        fclose(f);
        return -1;
    }
    c->li = c->ci = c->lf = c->cf = -1;

    char token[64];
    for (int i = 0; i < c->linhas; i++) {
        for (int j = 0; j < c->colunas; j++) {
            if (fscanf(f, "%63s", token) != 1) {
                fprintf(stderr, "Arquivo com menos células que o esperado.\n");
                fclose(f);
                return -1;
            }
            if (token[0] == 'I') {
                c->li = i; c->ci = j;
                c->valor[IDX(c, i, j)] = 0;
            } else if (token[0] == 'F') {
                c->lf = i; c->cf = j;
                c->valor[IDX(c, i, j)] = 0;
            } else {
                c->valor[IDX(c, i, j)] = strtoll(token, NULL, 10); /* aceita +20 e -20 */
            }
        }
    }
    fclose(f);

    if (c->li < 0 || c->lf < 0) {
        fprintf(stderr, "Mapa sem 'I' ou sem 'F'.\n");
        return -1;
    }
    return 0;
}

/* Resolve por PD e grava resultado.txt */
int resolverCimaEsq(const Caverna *c, const char *saida) {
    int n = c->linhas, m = c->colunas;
    size_t total = (size_t)n * m;

    long long *dp = (long long *)malloc(total * sizeof(long long));
    char *pai = (char *)calloc(total, sizeof(char)); /* 'B' = veio de baixo, 'D' = veio da direita */
    if (!dp || !pai) {
        fprintf(stderr, "Memória insuficiente.\n");
        free(dp); free(pai);
        return -1;
    }
    for (size_t k = 0; k < total; k++) dp[k] = INVALIDO;

    /* Preenchimento: de I em direção ao canto superior esquerdo.
       Só interessam as células com i <= li e j <= ci (cima/esquerda de I). */
    for (int i = c->li; i >= 0; i--) {
        for (int j = c->ci; j >= 0; j--) {
            if (i == c->li && j == c->ci) {
                dp[IDX(c, i, j)] = c->vidaInicial;
                continue;
            }

            long long melhor = INVALIDO;
            char origem = 0;

            /* veio de baixo (i+1, j), movimento "cima" */
            if (i + 1 <= c->li && dp[IDX(c, i + 1, j)] != INVALIDO) {
                melhor = dp[IDX(c, i + 1, j)];
                origem = 'B';
            }
            /* veio da direita (i, j+1), movimento "esquerda" */
            if (j + 1 <= c->ci && dp[IDX(c, i, j + 1)] != INVALIDO &&
                dp[IDX(c, i, j + 1)] > melhor) {
                melhor = dp[IDX(c, i, j + 1)];
                origem = 'D';
            }

            if (melhor == INVALIDO) continue; /* inalcançável */

            long long v = melhor + c->valor[IDX(c, i, j)];
            if (v > 0) {                      /* PV <= 0 => morreu */
                dp[IDX(c, i, j)] = v;
                pai[IDX(c, i, j)] = origem;
            }
        }
    }

    FILE *out = fopen(saida, "w");
    if (!out) {
        perror("Erro ao criar resultado.txt");
        free(dp); free(pai);
        return -1;
    }

    int alcancavel = (c->lf <= c->li && c->cf <= c->ci &&
                      dp[IDX(c, c->lf, c->cf)] != INVALIDO);

    if (!alcancavel) {
        fprintf(out, "impossível");
    } else {
        int passos = (c->li - c->lf) + (c->ci - c->cf) + 1;
        int *pl = (int *)malloc(passos * sizeof(int));
        int *pc = (int *)malloc(passos * sizeof(int));

        /* reconstrução: de F até I, depois imprime invertido */
        int i = c->lf, j = c->cf, k = 0;
        while (1) {
            pl[k] = i; pc[k] = j; k++;
            if (i == c->li && j == c->ci) break;
            if (pai[IDX(c, i, j)] == 'B') i++;
            else j++;
        }
        for (int t = k - 1; t >= 0; t--)
            fprintf(out, "%d %d\n", pl[t], pc[t]);

        free(pl); free(pc);
    }

    fclose(out);
    free(dp); free(pai);
    return 0;
}

void liberarCaverna(Caverna *c) {
    free(c->valor);
    c->valor = NULL;
}
