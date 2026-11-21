#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "guloso.h"
#include "arte.h"

typedef struct { int l, c; } Pos;

enum { G_CHEGOU = 1, G_MORREU = 0, G_SEM_CAMINHO = -1 };

/* Guloso: em cada celula escolhe, entre cima e esquerda, a de MAIOR valor
   (empate: cima). Para nao se afastar de F, so considera um movimento se ele
   mantem F alcancavel (linha >= lf e coluna >= cf). Para se os PV chegarem a 0.
   Retorna G_CHEGOU, G_MORREU ou G_SEM_CAMINHO (F nao esta acima/a esquerda de I).
   Preenche cam[0..*n-1] com o trajeto (inclusive a celula onde morreu). */
static int simularGuloso(const Caverna *c, Pos *cam, int *n, long long *pvFinal) {
    *n = 0;
    if (c->lf > c->li || c->cf > c->ci) 
        return G_SEM_CAMINHO;

    int l = c->li, col = c->ci;
    long long pv = c->vidaInicial;
    cam[(*n)++] = (Pos){l, col};

    while (l != c->lf || col != c->cf) {
        int podeCima = l > c->lf, podeEsq = col > c->cf;
        int vaiCima;
        if (podeCima && podeEsq)
            vaiCima = c->valor[IDX(c, l - 1, col)] >= c->valor[IDX(c, l, col - 1)];
        else
            vaiCima = podeCima;

        if (vaiCima) 
            l--; 
        else 
            col--;

        pv += c->valor[IDX(c, l, col)];
        cam[(*n)++] = (Pos){l, col};
        if (pv <= 0) { 
            *pvFinal = pv; 
            return G_MORREU; }
    }
    *pvFinal = pv;
    return G_CHEGOU;
}

/* Caminho otimo da PD, lido do arquivo gerado por resolverCimaEsq().
   Retorna 1 se ha solucao (cam/n preenchidos), 0 se impossivel, -1 em erro. */
static int caminhoPD(const Caverna *c, Pos *cam, int *n, long long *pvFinal) {
    const char *tmp = "resultado_pd.tmp";
    *n = 0;
    if (resolverCimaEsq(c, tmp) != 0) 
        return -1;
    FILE *f = fopen(tmp, "r");
    if (!f) 
        return -1;

    int l, col;
    long long pv = c->vidaInicial;
    while (fscanf(f, "%d %d", &l, &col) == 2) {
        if (*n > 0) 
            pv += c->valor[IDX(c, l, col)];
        cam[(*n)++] = (Pos){l, col};
    }
    fclose(f);
    remove(tmp);
    if (*n == 0) 
        return 0;
    *pvFinal = pv;
    return 1;
}

int resolverGuloso(const Caverna *c, long long *pvFinal) {
    Pos *cam = (Pos *)malloc(((size_t)c->li + c->ci + 2) * sizeof(Pos));
    if (!cam) 
        return G_SEM_CAMINHO;
    int n = 0;
    int r = simularGuloso(c, cam, &n, pvFinal);
    free(cam);
    return r;
}

/* Mostra a caverna com os desenhos, marcando o trajeto `cam` com o rastro. */
static void desenharCaminho(const Caverna *c, const char *titulo, const Pos *cam, int n) {
    char *vis = (char *)calloc((size_t)c->linhas * c->colunas, 1);
    if (!vis) 
        return;
    for (int k = 0; k < n; k++) 
        vis[IDX(c, cam[k].l, cam[k].c)] = 1;
    printf("\n--- %s ---\n", titulo);
    printarCaverna(c, vis, c->li, c->ci);
    free(vis);
}

int compararGulosoComPD(const Caverna *c, const char *saida) {
    size_t cap = (size_t)c->li + c->ci + 2;
    Pos *camG = (Pos *)malloc(cap * sizeof(Pos));
    Pos *camD = (Pos *)malloc(cap * sizeof(Pos));
    if (!camG || !camD) {
        fprintf(stderr, "Memoria insuficiente.\n");
        free(camG); free(camD);
        return -1;
    }

    int nG = 0, nD = 0;
    long long pvG = 0, pvD = 0;
    int rG = simularGuloso(c, camG, &nG, &pvG);
    int rD = caminhoPD(c, camD, &nD, &pvD);

    if (rD < 0) { 
        free(camG); 
        free(camD); 
        return -1; 
    }

    /* arquivo do guloso, no mesmo formato do resultado.txt */
    FILE *f = fopen(saida, "w");
    if (!f) {
        perror("Erro ao gravar a saida do guloso");
        free(camG); free(camD);
        return -1;
    }
    if (rG == G_CHEGOU) {
        for (int k = 0; k < nG; k++) 
            fprintf(f, "%d %d\n", camG[k].l, camG[k].c);
    } else {
        fprintf(f, "impossível");  /* o guloso nao conseguiu sair vivo */
    }
    fclose(f);

    printf("=== Guloso x Programacao Dinamica ===\n");
    printf("Caverna %dx%d, PV iniciais: %lld\n", c->linhas, c->colunas, c->vidaInicial);
    desenharCaminho(c, "Caminho da PD (otimo)", camD, nD);
    desenharCaminho(c, "Caminho do guloso", camG, nG);

    printf("\nPD     : ");
    if (rD == 1) 
        printf("saiu com %lld PV (%d passos)\n", pvD, nD - 1);
    else         
        printf("impossivel escapar\n");

    printf("Guloso : ");
    if (rG == G_CHEGOU)           
        printf("saiu com %lld PV (%d passos)\n", pvG, nG - 1);
    else if (rG == G_MORREU)      
        printf("morreu em (%d, %d) com %lld PV\n", camG[nG - 1].l, camG[nG - 1].c, pvG);
    else                          
        printf("impossivel escapar\n");

    printf("\nConclusao: ");
    if (rD != 1)
        printf("sem solucao, nem a PD escapa.\n");
    else if (rG != G_CHEGOU)
        printf("o guloso FALHOU, mas existe caminho com %lld PV (a PD acha).\n", pvD);
    else if (pvG == pvD)
        printf("o guloso achou o otimo (%lld PV) neste mapa.\n", pvD);
    else
        printf("o guloso e sub-otimo: perdeu %lld PV em relacao a PD.\n", pvD - pvG);

    printf("Custo: guloso O(n+m) | PD O(n*m).\n");
    printf("Caminho do guloso gravado em \"%s\".\n", saida);

    free(camG); 
    free(camD);
    return 0;
}