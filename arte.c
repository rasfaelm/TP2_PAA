#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arte.h"

#define pre  "\033[30m"
#define VERM "\033[31m"
#define ama  "\033[33m"
#define AZU  "\033[34m"
#define mag  "\033[35m"
#define cia  "\033[36m"
#define VERD "\033[32m"
#define NOR  "\033[0m"

void menu(void) {
    printf(VERM "                          )       \\   /      (                         \n");
    printf("                         /|\\      )\\_/(     /|\\                        \n" NOR);
    printf("*" VERM "                       / | \\    (/\\|/\\)   / | \\                      " NOR "*\n");
    printf("|`.____________________" VERM "/" NOR "__" VERM "|" NOR "__o____" VERM "\\" NOR AZU "`" NOR VERM "|" NOR AZU "'" NOR VERM "/" NOR "___o__" VERM "|" NOR "__" VERM "\\" NOR "___________________.'|\n");
    printf("|                           '^`    " VERM "\\|/" NOR "   '^`                          |\n");
    printf("|                                   " VERM "V" NOR "                                 |\n");
    printf("|                  _______      " VERM " ________  " NOR "  _______                  |\n");
    printf("|                 |   _   \\    " VERM " |       | " NOR "  |   _   \\                 |\n");
    printf("|                 |  | \\   \\  " VERM "  |   ____|" NOR "   |  | \\   \\                |\n");
    printf("|                 |  |  \\   |  " VERM " |  |____  " NOR "  |  |  \\   |               |\n");
    printf("|                 |  |   |  |   " VERM "|   ____|  " NOR " |  |   |  |               |\n");
    printf("|                 |  |  /   /   " VERM "|  |____   " NOR " |  |  /   /               |\n");
    printf("|                 |  |_/   /    " VERM "|       |  " NOR " |  |_/   /                |\n");
    printf("|                 |_______/     " VERM "|_______|  " NOR " |_______/                 |\n");
    printf("|                                                                     |\n");
    printf("|" AZU "                   Projeto e Analise de Algoritmos                   " NOR "|\n");
    printf("|                                                                     |\n");
    printf("| ._________________________________________________________________. |\n");
    printf("|'               " VERM "l    /\\ /     \\\\            \\ /\\   l" NOR "                `|\n");
    printf("*                " VERM "l  /   V       ))            V   \\ l" NOR "                 *\n");
    printf("                 " VERM "l/            //                  \\I" NOR "                  \n");
}


// Desenhos dos elementos
static const char *desenho(const char *tipo, int linha) {

    static const char *vazio[] = {
        "       ",
        "       ",
        "       ",
        "       ",
        "       "
    };

    static const char *mortoVivo[] = {
        "       ",
        "   o   ",
        "  ´|\\  ",
        "  / \\  ",
        "       "
    };

    static const char *personagem[] = {
        "       ",
        "   o   ",
        "  /|\\  ",
        "  / \\  ",
        "       "
    };

    static const char *pocao[] = {
        " ___  ",
        " |_|  ",
        " /_\\  ",
        "|* @| ",
        "\\___/ "
    };

    static const char *saida[] = {
        " ______",
        "|SAIDA|",
        "| ____|",
        "||     ",
        "||WVW  "
    };

    static const char *bixo[] = {
        " { O } ",
        "__| |__",
        "|| X ||",
        "mn X nm",
        " // \\\\ "
    };

    static const char *caminhoPassado[] = {
        "       ",
        "\\  /   ",
        " \\/    ",
        " /\\    ",
        "/  \\   "
    };


    if (strcmp(tipo, "-10") == 0)
        return mortoVivo[linha];

    else if (strcmp(tipo, "I") == 0)
        return personagem[linha];

    else if (strcmp(tipo, "+20") == 0)
        return pocao[linha];

    else if (strcmp(tipo, "F") == 0)
        return saida[linha];

    else if (strcmp(tipo, "-20") == 0)
        return bixo[linha];

    else if (strcmp(tipo, "2") == 0)
        return caminhoPassado[linha];

    else
        return vazio[linha];
}


// Impressão do mapa
void printarMapa(char ***mapa, int linhas, int colunas) {

    int i, j, linha;

    printf("\n----------+LABIRINTO+----------\n\n");

    for (i = 0; i < linhas; i++) {

        for (linha = 0; linha < 5; linha++) {

            for (j = 0; j < colunas; j++) {

                const char *figura = desenho(mapa[i][j], linha);

                if (strcmp(mapa[i][j], "-20") == 0 ||
                    strcmp(mapa[i][j], "-10") == 0)

                    printf(VERM "%s" NOR " ", figura);

                else if (strcmp(mapa[i][j], "+20") == 0)

                    printf(mag "%s" NOR " ", figura);

                else if (strcmp(mapa[i][j], "F") == 0)

                    printf(VERD "%s" NOR " ", figura);

                else if (strcmp(mapa[i][j], "I") == 0)

                    printf(AZU "%s" NOR " ", figura);

                else

                    printf("%s ", figura);
            }

            printf("\n");
        }

        printf("\n");
    }
}


/* teste de coração
/ \/ \
\    /
 \  /
  \/
*/