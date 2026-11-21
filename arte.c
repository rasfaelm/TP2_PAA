#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arte.h"

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

/* No Windows, liga cores ANSI e UTF-8 no console (nos outros sistemas nao faz nada). */
static void prepararConsole(void) {
#ifdef _WIN32
    static int pronto = 0;
    if (pronto) return;
    pronto = 1;
    SetConsoleOutputCP(CP_UTF8);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &modo))
        SetConsoleMode(h, modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

#define pre  "\033[30m"
#define VERM "\033[31m"
#define ama  "\033[33m"
#define AZU  "\033[34m"
#define mag  "\033[35m"
#define cia  "\033[36m"
#define VERD "\033[32m"
#define NOR  "\033[0m"

void menu(void) {
    prepararConsole();
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
        " ___   ",
        " |_|   ",
        " /_\\   ",
        "|* @|  ",
        "\\___/  "
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

    prepararConsole();

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


/* ---- Caverna do modo manual ---- */

/* Celula com valor sem desenho proprio: quadro vazio com o numero no meio. */
static void linhaNumero(long long v, int linha, char *out) {
    memset(out, ' ', 7);
    out[7] = '\0';
    if (linha != 2) return;
    char t[32];
    snprintf(t, sizeof t, "%+lld", v);
    int len = (int)strlen(t);
    if (len > 7) len = 7;
    memcpy(out + (7 - len) / 2, t, len);
}

void printarCaverna(const Caverna *c, const char *visitado, int pl, int pc) {
    prepararConsole();

    printf("    ");
    for (int j = 0; j < c->colunas; j++)
        printf("   %-5d", j);
    printf("\n");

    for (int i = 0; i < c->linhas; i++) {
        for (int linha = 0; linha < 5; linha++) {
            if (linha == 2) printf("%3d ", i);
            else            printf("    ");

            for (int j = 0; j < c->colunas; j++) {
                long long v = c->valor[IDX(c, i, j)];
                char tipo[32];
                const char *cor = "";
                char numero[8];
                const char *figura;

                if (i == pl && j == pc)                { strcpy(tipo, "I"); cor = AZU; }
                else if (i == c->lf && j == c->cf)     { strcpy(tipo, "F"); cor = VERD; }
                else if ((i == c->li && j == c->ci) || (visitado && visitado[IDX(c, i, j)]))
                                                       { strcpy(tipo, "2"); }
                else if (v == 0)                       { strcpy(tipo, "0"); }
                else {
                    snprintf(tipo, sizeof tipo, "%+lld", v);
                    if (v < 0) cor = VERM; else cor = mag;
                }

                if (strcmp(tipo, "-20") == 0 || strcmp(tipo, "-10") == 0 ||
                    strcmp(tipo, "+20") == 0 || strcmp(tipo, "I") == 0 ||
                    strcmp(tipo, "F") == 0   || strcmp(tipo, "2") == 0 ||
                    strcmp(tipo, "0") == 0) {
                    figura = desenho(tipo, linha);
                } else {
                    linhaNumero(v, linha, numero);
                    figura = numero;
                }
                printf("%s%s" NOR " ", cor, figura);
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