/* Extra 5: mede tempo x tamanho da entrada da PD (cima/esquerda).
 *
 * Uso: ./bench [n_max] [passo] [repeticoes] [saida.csv]
 *   defaults: 2000 200 5 bench.csv
 *
 * Para cada n em {passo, 2*passo, ..., n_max} gera uma caverna n x n
 * aleatoria com I no canto inferior direito e F no superior esquerdo
 * (pior caso: a tabela da PD inteira é preenchida). Semente fixa => resultados reprodutiveis.
 * Só o tempo de resolverCimaEsq() (PD) e de resolverGuloso() é medido (não medi geração aqui).
 * O guloso é tão rápido (O(n+m)) que roda REPS_GULOSO vezes por tamanho para a média ser confiável.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "caverna.h"
#include "guloso.h"

#define REPS_GULOSO 2000

static unsigned long long rng_estado = 123456789ULL;
static unsigned int prox(void) {            /* xorshift simples e portavel */
    rng_estado ^= rng_estado << 13;
    rng_estado ^= rng_estado >> 7;
    rng_estado ^= rng_estado << 17;
    return (unsigned int)(rng_estado >> 11);
}

static int gerar(Caverna *c, int n) {
    c->linhas = c->colunas = n;
    c->vidaInicial = 1000000000LL;          /* alto: ninguem morre */
    c->valor = (long long *)malloc((size_t)n * n * sizeof(long long));
    if (!c->valor) return -1;
    for (size_t k = 0; k < (size_t)n * n; k++)
        c->valor[k] = (long long)(prox() % 31) - 10;
    c->li = n - 1; c->ci = n - 1;
    c->lf = 0;     c->cf = 0;
    c->valor[IDX(c, c->li, c->ci)] = 0;
    c->valor[IDX(c, c->lf, c->cf)] = 0;
    return 0;
}

int main(int argc, char **argv) {
    int nmax  = argc > 1 ? atoi(argv[1]) : 2000;
    int passo = argc > 2 ? atoi(argv[2]) : 200;
    int reps  = argc > 3 ? atoi(argv[3]) : 5;
    const char *csv = argc > 4 ? argv[4] : "bench.csv";
    if (nmax < 1 || passo < 1 || reps < 1) {
        fprintf(stderr, "Uso: %s [n_max] [passo] [repeticoes] [saida.csv]\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(csv, "w");
    if (!f) { perror("Erro ao criar CSV"); return 1; }
    fprintf(f, "n,celulas,tempo_ms,tempo_guloso_ms\n");
    printf("%8s %12s %12s %16s\n", "n", "celulas", "tempo_ms", "guloso_ms");

    for (int n = passo; n <= nmax; n += passo) {
        Caverna c;
        if (gerar(&c, n) != 0) { fprintf(stderr, "Sem memoria para n=%d\n", n); break; }

        double total = 0.0;
        for (int r = 0; r < reps; r++) {
            clock_t t0 = clock();
            if (resolverCimaEsq(&c, "bench_saida.tmp") != 0) { liberarCaverna(&c); fclose(f); return 1; }
            total += 1000.0 * (double)(clock() - t0) / CLOCKS_PER_SEC;
        }
        double media = total / reps;

        volatile long long sink = 0;   /* impede o compilador de descartar a chamada */
        clock_t g0 = clock();
        for (int r = 0; r < REPS_GULOSO; r++) {
            long long pv = 0;
            resolverGuloso(&c, &pv);
            sink += pv;
        }
        double mediaG = 1000.0 * (double)(clock() - g0) / CLOCKS_PER_SEC / REPS_GULOSO;
        (void)sink;

        printf("%8d %12lld %12.3f %16.6f\n", n, (long long)n * n, media, mediaG);
        fprintf(f, "%d,%lld,%.3f,%.6f\n", n, (long long)n * n, media, mediaG);
        liberarCaverna(&c);
    }
    fclose(f);
    remove("bench_saida.tmp");
    printf("\nCSV gravado em %s\n", csv);
    return 0;
}