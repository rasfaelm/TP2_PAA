/* 
 *   ./caverna <mapa.txt> [saida.txt]   resolve e grava o caminho (padrao: resultado.txt)
 *   ./caverna --criar [mapa.txt]       cria um mapa digitando no terminal e salva (padrao: caverna_criada.txt)
 */
#include <stdio.h>
#include <string.h>

#include "caverna.h"
#include "mapa_manual.h"

#define SAIDA_PADRAO "resultado.txt"
#define MAPA_CRIADO_PADRAO "caverna_criada.txt"

static void ajuda(const char *prog) {
    printf("Uso:\n");
    printf("  %s <mapa.txt> [saida.txt]  resolve o mapa (saida padrao: %s)\n", prog, SAIDA_PADRAO);
    printf("  %s --criar [mapa.txt]      cria um mapa manualmente (padrao: %s)\n", prog, MAPA_CRIADO_PADRAO);
    printf("  %s --ajuda                 mostra esta mensagem\n", prog);
}

static int modoResolver(const char *mapa, const char *saida) {
    Caverna c;
    if (lerCaverna(mapa, &c) != 0) return 1;
    int r = resolverCimaEsq(&c, saida);
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

    return modoResolver(argv[1], argc >= 3 ? argv[2] : SAIDA_PADRAO);
}