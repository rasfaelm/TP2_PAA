#ifndef CAVERNA_H
#define CAVERNA_H

typedef struct {
    int linhas, colunas;
    long long vidaInicial;
    long long *valor;   /* matriz linearizada: variacao de PV de cada celula */
    int li, ci;         /* posicao de I (inicio) */
    int lf, cf;         /* posicao de F (fim)    */
} Caverna;

#define IDX(cav, i, j) ((size_t)(i) * (cav)->colunas + (j))

/* Le o arquivo de entrada. Retorna 0 em sucesso, -1 em erro. */
int lerCaverna(const char *caminho, Caverna *c);

/* Resolve por PD (so cima/esquerda) e grava o caminho em `saida`
   (ou "impossível"). Retorna 0 em sucesso, -1 em erro. */
int resolverCimaEsq(const Caverna *c, const char *saida);

/* Libera a memoria da caverna. */
void liberarCaverna(Caverna *c);

#endif
