/* 
 *   ./caverna <mapa.txt> [saida.txt]   resolve e grava o caminho (padrao: resultado.txt)
 *   ./caverna --criar [mapa.txt]       cria um mapa digitando no terminal e salva (padrao: caverna_criada.txt)
 *   ./caverna --guloso <mapa.txt> [saida.txt]  resolve com o guloso e compara com a PD (padrao: resultado_guloso.txt)
 *   ./caverna --jogar <mapa.txt> [saida.txt]  joga o mapa movendo o personagem pelo teclado (padrao: caminho_manual.txt)
 */
#include <stdio.h>
#include <string.h>

#include "caverna.h"
#include "mapa_manual.h"
#include "jogo_manual.h"
#include "guloso.h"

#define SAIDA_PADRAO "resultado.txt"
#define MAPA_CRIADO_PADRAO "caverna_criada.txt"
#define CAMINHO_MANUAL_PADRAO "caminho_manual.txt"
#define SAIDA_GULOSO_PADRAO "resultado_guloso.txt"

static void ajuda(const char *prog) {
    printf("Uso:\n");
    printf("  %s <mapa.txt> [saida.txt]  resolve o mapa (saida padrao: %s)\n", prog, SAIDA_PADRAO);
    printf("  %s --criar [mapa.txt]      cria um mapa manualmente (padrao: %s)\n", prog, MAPA_CRIADO_PADRAO);
    printf("  %s --jogar <mapa.txt> [saida.txt]  joga o mapa pelo teclado (padrao: %s)\n", prog, CAMINHO_MANUAL_PADRAO);
    printf("  %s --guloso <mapa.txt> [saida.txt]  guloso x PD (padrao: %s)\n", prog, SAIDA_GULOSO_PADRAO);
    printf("  %s --ajuda                 mostra esta mensagem\n", prog);
}

static int modoResolver(const char *mapa, const char *saida) {
    Caverna c;
    if (lerCaverna(mapa, &c) != 0) return 1;
    int r = resolverCimaEsq(&c, saida);
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
    int r = salvarCaverna(&c, destino);
    liberarCaverna(&c);
    if (r != 0) return 1;
    printf("\nMapa salvo em \"%s\". Para resolver: ./caverna %s\n", destino, destino);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2 || strcmp(argv[1], "--ajuda") == 0 || strcmp(argv[1], "-h") == 0) {
        ajuda(argv[0]);
        return argc < 2 ? 1 : 0;
    }
    if (strcmp(argv[1], "--criar") == 0)
        return modoCriar(argc >= 3 ? argv[2] : MAPA_CRIADO_PADRAO);

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