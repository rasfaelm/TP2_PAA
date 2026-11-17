#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mapa_manual.h"

/* Lê um inteiro >= min da entrada padrao, repetindo ate ser valido. */
static int pedirInteiro(const char *rotulo, long long min, long long *out) {
    char buf[128];
    for (;;) {
        printf("%s", rotulo);
        fflush(stdout);
        if (!fgets(buf, sizeof buf, stdin)) return -1; /* EOF */
        char *fim;
        long long v = strtoll(buf, &fim, 10);
        while (*fim == ' ' || *fim == '\t' || *fim == '\r' || *fim == '\n') fim++;
        if (fim == buf || *fim != '\0') {
            printf("  Valor inválido. Digite um numero inteiro.\n");
            continue;
        }
        if (v < min) {
            printf("  O valor deve ser >= %lld.\n", min);
            continue;
        }
        *out = v;
        return 0;
    }
}

/* Interpreta uma linha da matriz. Preenche vals[0..colunas-1] e marca
   posicoes de I/F em *colI / *colF (-1 se nao houver nesta linha).*/
static int interpretarLinha(char *linha, int colunas, long long *vals, int *colI, int *colF) {
    int j = 0;
    *colI = *colF = -1;
    for (char *tok = strtok(linha, " \t\r\n"); tok; tok = strtok(NULL, " \t\r\n")) {
        if (j >= colunas) {
            printf("  Células demais: esperado %d.\n", colunas);
            return -1;
        }
        if (strlen(tok) == 1 && toupper((unsigned char)tok[0]) == 'I') {
            if (*colI >= 0) { printf("  Mais de um 'I' na linha.\n"); return -1; }
            *colI = j; vals[j] = 0;
        } else if (strlen(tok) == 1 && toupper((unsigned char)tok[0]) == 'F') {
            if (*colF >= 0) { printf("  Mais de um 'F' na linha.\n"); return -1; }
            *colF = j; vals[j] = 0;
        } else {
            char *fim;
            long long v = strtoll(tok, &fim, 10);
            if (fim == tok || *fim != '\0') {
                printf("  Token invalido: \"%s\" (use inteiro, I ou F).\n", tok);
                return -1;
            }
            vals[j] = v;
        }
        j++;
    }
    if (j != colunas) {
        printf("  Celulas de menos: recebi %d, esperado %d.\n", j, colunas);
        return -1;
    }
    return 0;
}

int criarCavernaManual(Caverna *c) {
    long long n, m, vida;
    printf("=== Criacao manual de caverna ===\n");
    if (pedirInteiro("Numero de linhas: ", 1, &n) < 0) return -1;
    if (pedirInteiro("Numero de colunas: ", 1, &m) < 0) return -1;
    if (pedirInteiro("Pontos de vida iniciais (>= 1): ", 1, &vida) < 0) return -1;
    if (n > 100000 || m > 100000 || n * m > 100000000LL) {
        fprintf(stderr, "Caverna grande demais para criar manualmente.\n");
        return -1;
    }

    c->linhas = (int)n;
    c->colunas = (int)m;
    c->vidaInicial = vida;
    c->li = c->ci = c->lf = c->cf = -1;
    c->valor = (long long *)calloc((size_t)n * m, sizeof(long long));
    if (!c->valor) { fprintf(stderr, "Memoria insuficiente.\n"); return -1; }

    size_t tam = (size_t)m * 32 + 64;
    char *linha  = (char *)malloc(tam);
    char *copia  = (char *)malloc(tam);
    long long *vals = (long long *)malloc((size_t)m * sizeof(long long));
    if (!linha || !copia || !vals) {
        fprintf(stderr, "Memoria insuficiente.\n");
        free(linha); free(copia); free(vals);
        liberarCaverna(c);
        return -1;
    }

    printf("\nDigite cada linha com %lld células separadas por espaço.\n", m);
    printf("Valores: inteiros (ex.: -20, 0, +20), I = inicio, F = fim.\n\n");

    for (int i = 0; i < c->linhas; i++) {
        for (;;) {
            printf("Linha %d: ", i);
            fflush(stdout);
            if (!fgets(linha, (int)tam, stdin)) {
                free(linha); free(copia); free(vals);
                liberarCaverna(c);
                return -1;
            }
            strcpy(copia, linha);
            int colI, colF;
            if (interpretarLinha(copia, c->colunas, vals, &colI, &colF) < 0) continue;

            if ((colI >= 0 && c->li >= 0) || (colF >= 0 && c->lf >= 0)) {
                printf("  'I' e 'F' devem aparecer apenas uma vez no mapa.\n");
                continue;
            }
            if (i == c->linhas - 1 &&
                ((c->li < 0 && colI < 0) || (c->lf < 0 && colF < 0))) {
                printf("  Falta 'I' ou 'F' no mapa; coloque nesta linha.\n");
                continue;
            }
            /* linha aceita: grava */
            for (int j = 0; j < c->colunas; j++) c->valor[IDX(c, i, j)] = vals[j];
            if (colI >= 0) { c->li = i; c->ci = colI; }
            if (colF >= 0) { c->lf = i; c->cf = colF; }
            break;
        }
    }
    free(linha); free(copia); free(vals);
    return 0;
}

int salvarCaverna(const Caverna *c, const char *caminho) {
    FILE *f = fopen(caminho, "w");
    if (!f) { perror("Erro ao criar arquivo de mapa"); return -1; }
    fprintf(f, "%d %d %lld\n", c->linhas, c->colunas, c->vidaInicial);
    for (int i = 0; i < c->linhas; i++) {
        for (int j = 0; j < c->colunas; j++) {
            if (j) fputc(' ', f);
            if (i == c->li && j == c->ci) fputc('I', f);
            else if (i == c->lf && j == c->cf) fputc('F', f);
            else fprintf(f, "%lld", c->valor[IDX(c, i, j)]);
        }
        fputc('\n', f);
    }
    fclose(f);
    return 0;
}