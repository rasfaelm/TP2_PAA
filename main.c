/* 
 *   ./caverna                          abre o menu interativo (igual a --menu)
 *   ./caverna <mapa.txt> [saida.txt]   resolve e grava o caminho (padrao: resultado.txt)
 *   ./caverna --criar [mapa.txt]       cria um mapa digitando no terminal e salva (padrao: caverna_criada.txt)
 *   ./caverna --guloso <mapa.txt> [saida.txt]  resolve com o guloso e compara com a PD (padrao: resultado_guloso.txt)
 *   ./caverna --jogar <mapa.txt> [saida.txt]  joga o mapa movendo o personagem pelo teclado (padrao: caminho_manual.txt)
 *   ./caverna --gerar <linhas> <colunas> <monstros-20> <monstros-10> <pocoes> <vida> [mapa.txt]
 *                                     gera um mapa aleatorio, mostra com os desenhos e salva (padrao: mapa_gerado.txt)
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "caverna.h"
#include "mapa_manual.h"
#include "jogo_manual.h"
#include "guloso.h"
#include "GeradorDeMapa.h"
#include "arte.h"

#define SAIDA_PADRAO "resultado.txt"
#define MAPA_CRIADO_PADRAO "testes/caverna_criada.txt"
#define CAMINHO_MANUAL_PADRAO "caminho_manual.txt"
#define SAIDA_GULOSO_PADRAO "resultado_guloso.txt"
#define MAPA_GERADO_PADRAO "testes/mapa_gerado.txt"

static void ajuda(const char *prog) {
    printf("Uso:\n");
    printf("  %s [--menu]                abre o menu interativo\n", prog);
    printf("  %s <mapa.txt> [saida.txt]  resolve o mapa (saida padrao: %s)\n", prog, SAIDA_PADRAO);
    printf("  %s --criar [mapa.txt]      cria um mapa manualmente (padrao: %s)\n", prog, MAPA_CRIADO_PADRAO);
    printf("  %s --jogar <mapa.txt> [saida.txt]  joga o mapa pelo teclado (padrao: %s)\n", prog, CAMINHO_MANUAL_PADRAO);
    printf("  %s --guloso <mapa.txt> [saida.txt]  guloso x PD (padrao: %s)\n", prog, SAIDA_GULOSO_PADRAO);
    printf("  %s --gerar <linhas> <colunas> <monstros-20> <monstros-10> <pocoes> <vida> [mapa.txt]\n", prog);
    printf("                             gera um mapa aleatorio e mostra com desenhos (padrao: %s)\n", MAPA_GERADO_PADRAO);
    printf("  %s --ajuda                 mostra esta mensagem\n", prog);
}

/* Desenha a caverna com o caminho gravado em `arquivo` e resume o resultado. */
static void mostrarSolucao(const Caverna *c, const char *arquivo) {
    char *vis = (char *)calloc((size_t)c->linhas * c->colunas, 1);
    FILE *f = fopen(arquivo, "r");
    int l, col, n = 0;
    long long pv = c->vidaInicial;

    if (f && vis) {
        while (fscanf(f, "%d %d", &l, &col) == 2) {
            if (n > 0) pv += c->valor[IDX(c, l, col)];
            vis[IDX(c, l, col)] = 1;
            n++;
        }
    }
    if (f) fclose(f);

    printf("\n=== Caverna %dx%d, PV iniciais: %lld ===\n", c->linhas, c->colunas, c->vidaInicial);
    printarCaverna(c, vis, c->li, c->ci);
    if (n > 0)
        printf("\nMelhor caminho: sai com %lld PV (%d passos). Gravado em \"%s\".\n", pv, n - 1, arquivo);
    else
        printf("\nImpossivel escapar com vida. Resultado gravado em \"%s\".\n", arquivo);
    free(vis);
}

static int modoResolver(const char *mapa, const char *saida) {
    Caverna c;
    if (lerCaverna(mapa, &c) != 0) return 1;
    int r = resolverCimaEsq(&c, saida);
    if (r == 0) mostrarSolucao(&c, saida);
    liberarCaverna(&c);
    return r == 0 ? 0 : 1;
}

static int modoJogar(const char *mapa, const char *saida) {
    Caverna c;
    if (lerCaverna(mapa, &c) != 0) return 1;
    int r = jogarCaverna(&c, saida);
    liberarCaverna(&c);
    return r == 0 ? 0 : 1;
}

static int modoGuloso(const char *mapa, const char *saida) {
    Caverna c;
    if (lerCaverna(mapa, &c) != 0) return 1;
    int r = compararGulosoComPD(&c, saida);
    liberarCaverna(&c);
    return r == 0 ? 0 : 1;
}

static int modoCriar(const char *destino) {
    Caverna c;
    if (criarCavernaManual(&c) != 0) return 1;
    printf("\n=== Mapa criado ===\n");
    printarCaverna(&c, NULL, c.li, c.ci);
    int r = salvarCaverna(&c, destino);
    liberarCaverna(&c);
    if (r != 0) return 1;
    printf("\nMapa salvo em \"%s\". Para resolver: ./caverna %s\n", destino, destino);
    return 0;
}

/* Converte um argumento em inteiro >= min. Retorna 0 em sucesso. */
static int lerArgInt(const char *s, const char *nome, long long min, long long *out) {
    char *fim;
    long long v = strtoll(s, &fim, 10);
    if (fim == s || *fim != '\0' || v < min) {
        fprintf(stderr, "Valor invalido para %s: \"%s\" (inteiro >= %lld).\n", nome, s, min);
        return -1;
    }
    *out = v;
    return 0;
}

/* Gera, mostra (com os desenhos) e salva um mapa aleatorio. Retorna 0 em sucesso. */
static int gerarMapa(long long linhas, long long colunas, long long m1, long long m2,
                     long long pocoes, long long vida, const char *destino) {
    if (linhas > 1000 || colunas > 1000) {
        fprintf(stderr, "Mapa grande demais para gerar (maximo 1000x1000).\n");
        return 1;
    }

    srand((unsigned)time(NULL));
    char ***mapa = GeraMapa((int)m1, (int)m2, (int)pocoes, (int)linhas, (int)colunas);
    if (!mapa) {
        fprintf(stderr, "Nao foi possivel gerar o mapa: os %lld elementos + I + F nao cabem em %lldx%lld celulas.\n",
                m1 + m2 + pocoes, linhas, colunas);
        return 1;
    }

    printarMapa(mapa, (int)linhas, (int)colunas);

    Caverna c;
    int r = MapaParaCaverna(mapa, (int)linhas, (int)colunas, vida, &c);
    if (r == 0) {
        r = salvarCaverna(&c, destino);
        if (r == 0)
            printf("\nMapa salvo em \"%s\" (vida inicial: %lld). Para resolver: ./caverna %s\n", destino, vida, destino);
        liberarCaverna(&c);
    }
    LiberaMapa(mapa, (int)linhas, (int)colunas);
    return r == 0 ? 0 : 1;
}

static int modoGerar(char **a, int n) {
    /* a[0..5] = linhas colunas m1 m2 pocoes vida ; a[6] = arquivo (opcional) */
    long long linhas, colunas, m1, m2, pocoes, vida;
    if (n < 6) return -1;
    if (lerArgInt(a[0], "linhas", 1, &linhas) || lerArgInt(a[1], "colunas", 1, &colunas) ||
        lerArgInt(a[2], "monstros-20", 0, &m1) || lerArgInt(a[3], "monstros-10", 0, &m2) ||
        lerArgInt(a[4], "pocoes", 0, &pocoes) || lerArgInt(a[5], "vida", 1, &vida))
        return 1;
    return gerarMapa(linhas, colunas, m1, m2, pocoes, vida, n >= 7 ? a[6] : MAPA_GERADO_PADRAO);
}

static void limparTelaMenu(void) {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

/* Le uma linha de texto. Linha vazia usa `padrao` (se houver). -1 em EOF. */
static int pedirTexto(const char *rotulo, const char *padrao, char *buf, size_t tam) {
    for (;;) {
        if (padrao) printf("%s [%s]: ", rotulo, padrao);
        else        printf("%s: ", rotulo);
        fflush(stdout);
        if (!fgets(buf, (int)tam, stdin)) return -1;

        /* tira espacos, fim de linha e aspas (ao arrastar arquivo para o terminal) */
        size_t n = strlen(buf);
        while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r' || buf[n-1] == ' ' ||
                         buf[n-1] == '"'  || buf[n-1] == '\'')) buf[--n] = '\0';
        char *ini = buf;
        while (*ini == ' ' || *ini == '"' || *ini == '\'') ini++;
        if (ini != buf) memmove(buf, ini, strlen(ini) + 1);

        if (buf[0] == '\0') {
            if (!padrao) { printf("  Digite um valor.\n"); continue; }
            snprintf(buf, tam, "%s", padrao);
        }
        return 0;
    }
}

/* Le um inteiro >= min. -1 em EOF. */
static int pedirNumero(const char *rotulo, long long min, long long *out) {
    char buf[128];
    for (;;) {
        printf("%s: ", rotulo);
        fflush(stdout);
        if (!fgets(buf, sizeof buf, stdin)) return -1;
        char *fim;
        long long v = strtoll(buf, &fim, 10);
        while (*fim == ' ' || *fim == '\t' || *fim == '\r' || *fim == '\n') fim++;
        if (fim == buf || *fim != '\0') { printf("  Digite um numero inteiro.\n"); continue; }
        if (v < min) { printf("  O valor deve ser >= %lld.\n", min); continue; }
        *out = v;
        return 0;
    }
}

static void esperarEnter(void) {
    char buf[64];
    printf("\nPressione Enter para voltar ao menu...");
    fflush(stdout);
    if (!fgets(buf, sizeof buf, stdin)) return;
}

static void mostrarMenu(void) {
    limparTelaMenu();
    menu();
    printf("\n");
    printf("   [1] Resolver um mapa (programacao dinamica)\n");
    printf("   [2] Jogar um mapa pelo teclado\n");
    printf("   [3] Criar um mapa manualmente\n");
    printf("   [4] Gerar um mapa aleatorio\n");
    printf("   [5] Comparar guloso x programacao dinamica\n");
    printf("   [0] Sair\n\n");
    printf("   Escolha uma opcao: ");
    fflush(stdout);
}

/* Pergunta mapa e arquivo de saida; devolve 0 se ambos foram lidos. */
static int pedirMapaESaida(const char *saidaPadrao, char *mapa, size_t tm, char *saida, size_t ts) {
    printf("\n");
    if (pedirTexto("Arquivo do mapa", "testes/caverna1.txt", mapa, tm) < 0) return -1;
    if (pedirTexto("Arquivo de saida", saidaPadrao, saida, ts) < 0) return -1;
    printf("\n");
    return 0;
}

static int modoMenu(void) {
    char opcao[64], mapa[512], saida[512];

    for (;;) {
        mostrarMenu();
        if (!fgets(opcao, sizeof opcao, stdin)) { printf("\n"); return 0; }
        const char *p = opcao;
        while (*p == ' ' || *p == '\t') p++;
        char o = *p;
        if (o == '\n' || o == '\r' || o == '\0') continue;

        if (o == '0' || o == 'q' || o == 'Q') {
            limparTelaMenu();
            printf("Ate logo!\n");
            return 0;
        }

        limparTelaMenu();
        if (o == '1') {
            printf("=== Resolver mapa ===\n");
            if (pedirMapaESaida(SAIDA_PADRAO, mapa, sizeof mapa, saida, sizeof saida) < 0) return 0;
            modoResolver(mapa, saida);
        } else if (o == '2') {
            printf("=== Jogar mapa ===\n");
            if (pedirMapaESaida(CAMINHO_MANUAL_PADRAO, mapa, sizeof mapa, saida, sizeof saida) < 0) return 0;
            modoJogar(mapa, saida);
        } else if (o == '3') {
            printf("=== Criar mapa ===\n");
            if (pedirTexto("Salvar em", MAPA_CRIADO_PADRAO, saida, sizeof saida) < 0) return 0;
            printf("\n");
            modoCriar(saida);
        } else if (o == '4') {
            long long l, c, m1, m2, p, v;
            printf("=== Gerar mapa aleatorio ===\n\n");
            if (pedirNumero("Numero de linhas", 1, &l) < 0 ||
                pedirNumero("Numero de colunas", 1, &c) < 0 ||
                pedirNumero("Monstros de -20", 0, &m1) < 0 ||
                pedirNumero("Monstros de -10", 0, &m2) < 0 ||
                pedirNumero("Pocoes de +20", 0, &p) < 0 ||
                pedirNumero("Pontos de vida iniciais", 1, &v) < 0 ||
                pedirTexto("Salvar em", MAPA_GERADO_PADRAO, saida, sizeof saida) < 0)
                return 0;
            gerarMapa(l, c, m1, m2, p, v, saida);
        } else if (o == '5') {
            printf("=== Guloso x Programacao Dinamica ===\n");
            if (pedirMapaESaida(SAIDA_GULOSO_PADRAO, mapa, sizeof mapa, saida, sizeof saida) < 0) return 0;
            modoGuloso(mapa, saida);
        } else {
            printf("Opcao invalida.\n");
        }
        esperarEnter();
    }
}

int main(int argc, char **argv) {
    if (argc < 2 || strcmp(argv[1], "--menu") == 0)
        return modoMenu();

    if (strcmp(argv[1], "--ajuda") == 0 || strcmp(argv[1], "-h") == 0) {
        ajuda(argv[0]);
        return 0;
    }
    if (strcmp(argv[1], "--criar") == 0)
        return modoCriar(argc >= 3 ? argv[2] : MAPA_CRIADO_PADRAO);

    if (strcmp(argv[1], "--gerar") == 0) {
        int r = modoGerar(argv + 2, argc - 2);
        if (r < 0) { ajuda(argv[0]); return 1; }
        return r;
    }

    if (strcmp(argv[1], "--guloso") == 0) {
        if (argc < 3) { ajuda(argv[0]); return 1; }
        return modoGuloso(argv[2], argc >= 4 ? argv[3] : SAIDA_GULOSO_PADRAO);
    }

    if (strcmp(argv[1], "--jogar") == 0) {
        if (argc < 3) { ajuda(argv[0]); return 1; }
        return modoJogar(argv[2], argc >= 4 ? argv[3] : CAMINHO_MANUAL_PADRAO);
    }

    return modoResolver(argv[1], argc >= 3 ? argv[2] : SAIDA_PADRAO);
}