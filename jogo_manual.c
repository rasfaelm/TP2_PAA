#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <signal.h>
    #include <sys/select.h>
#endif

#include "jogo_manual.h"
#include "arte.h"

typedef struct {
    int l, c;          /* posicao */
    long long pv;      /* pontos de vida ao chegar nesta posicao */
} Passo;

/* Entrada de teclado sem Enter (estilo jogo) */

#ifdef _WIN32

static void modoTeclado(int ativar) { (void)ativar; }

static int lerTecla(void) {
    int t = _getch();
    if (t == 0 || t == 224) {               /* setas: prefixo + codigo */
        int u = _getch();
        if (u == 72) 
            return 'w';            /* seta cima     */
        if (u == 75) 
            return 'a';            /* seta esquerda */
        if (u == 80 || u == 77) 
            return 's'; /* baixo/direita: nao permitido */
        return 0;
    }
    return t;
}

static int interativo(void) { 
    return 1; 
}

static void limparTela(void) { 
    system("cls"); 
}

#else

static struct termios termiosAntigo;
static int termiosAtivo = 0;

static void restaurarTeclado(void) {
    if (termiosAtivo) {
        tcsetattr(STDIN_FILENO, TCSANOW, &termiosAntigo);
        termiosAtivo = 0;
    }
}

static void tratarSigint(int sig) {
    (void)sig;
    restaurarTeclado();
    _exit(130);
}

/* Liga/desliga o modo "tecla direta" (sem Enter e sem eco). */
static void modoTeclado(int ativar) {
    if (!ativar) { 
        restaurarTeclado(); 
        return; 
    }
    struct termios novo;
    if (tcgetattr(STDIN_FILENO, &termiosAntigo) != 0) 
        return; /* nao e terminal */
    novo = termiosAntigo;
    novo.c_lflag &= ~(ICANON | ECHO);
    novo.c_cc[VMIN] = 1;
    novo.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &novo) == 0) {
        termiosAtivo = 1;
        signal(SIGINT, tratarSigint);
    }
}

static int lerByte(void) {
    unsigned char ch;
    return read(STDIN_FILENO, &ch, 1) == 1 ? ch : -1;
}

static int temMaisBytes(void) {
    fd_set f; struct timeval tv = {0, 50000};
    FD_ZERO(&f); FD_SET(STDIN_FILENO, &f);
    return select(STDIN_FILENO + 1, &f, NULL, NULL, &tv) > 0;
}

static int lerTecla(void) {
    int t = lerByte();
    if (t == 27 && temMaisBytes()) {        /* setas: ESC [ A/B/C/D */
        if (lerByte() == '[') {
            int d = lerByte();
            if (d == 'A') 
                return 'w';
            if (d == 'D') 
                return 'a';
            if (d == 'B' || d == 'C')  
                return 's';
        }
        return 0;
    }
    return t;
}

static int interativo(void) { 
    return termiosAtivo; 
}

static void limparTela(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

#endif

/* Desenho */

static void desenharTela(const Caverna *c, const char *visitado, const Passo *at, const char *msg) {
    if (interativo()) 
        limparTela();

    printf("=== MODO MANUAL ===\n");
    printarCaverna(c, visitado, at->l, at->c);

    printf("Posicao: (%d, %d)   Pontos de vida: %lld\n", at->l, at->c, at->pv);
    printf("W/seta cima: cima | A/seta esquerda: esquerda | Z: desfazer | R: reiniciar | Q: sair\n");
    if (msg[0]) 
        printf("\n%s\n", msg);
    fflush(stdout);
}

/* Comparacao com o otimo da PD */

/* Retorna 1 e preenche *otimo se houver solucao; 0 se impossivel; -1 em erro. */
static int vidaOtima(const Caverna *c, long long *otimo) {
    const char *tmp = "resultado_otimo.tmp";
    
    if (resolverCimaEsq(c, tmp) != 0) 
        return -1;

    FILE *f = fopen(tmp, "r");

    if (!f) 
        return -1;

    long long pv = c->vidaInicial;
    int l, col, primeiro = 1, achou = 0;

    while (fscanf(f, "%d %d", &l, &col) == 2) {
        achou = 1;

        if (!primeiro) 
            pv += c->valor[IDX(c, l, col)];

        primeiro = 0;
    }

    fclose(f);
    remove(tmp);

    if (!achou) 
        return 0;

    *otimo = pv;

    return 1;
}

static int salvarCaminho(const Passo *p, int n, const char *saida) {
    FILE *f = fopen(saida, "w");

    if (!f) { 
        perror("Erro ao gravar o caminho"); 
        return -1; 
    }

    for (int k = 0; k < n; k++) 
        fprintf(f, "%d %d\n", p[k].l, p[k].c);

    fclose(f);
    return 0;
}

/* Loop do jogo */

int jogarCaverna(const Caverna *c, const char *saida) {
    
    size_t total = (size_t)c->linhas * c->colunas;
    char *visitado = (char *)calloc(total, 1);
    
    /* so ha movimentos para cima/esquerda: no maximo li + ci + 1 posicoes */
    Passo *caminho = (Passo *)malloc(((size_t)c->li + c->ci + 2) * sizeof(Passo));
    
    if (!visitado || !caminho) {
        fprintf(stderr, "Memoria insuficiente.\n");
        free(visitado); free(caminho);
        return -1;
    }

    int n = 0;
    caminho[n++] = (Passo){c->li, c->ci, c->vidaInicial};
    visitado[IDX(c, c->li, c->ci)] = 1;

    char msg[160] = "";
    int fim = 0, sair = 0;

    modoTeclado(1);
    
    while (!fim && !sair) {
        Passo *at = &caminho[n - 1];
        desenharTela(c, visitado, at, msg);
        msg[0] = '\0';

        int t = lerTecla();
        if (t == -1) 
            break;  /* EOF */
        
        char ch = (char)tolower((unsigned char)t);
        
        if (ch == '\n' || ch == '\r' || ch == ' ') 
            continue;

        if (ch == 'w' || ch == 'a') {
            if (at->pv <= 0) {
                snprintf(msg, sizeof msg, "Voce esta morto! Use Z para desfazer ou R para reiniciar.");
                continue;
            }
            int nl = at->l - (ch == 'w');
            int nc = at->c - (ch == 'a');
            if (nl < 0 || nc < 0) {
                snprintf(msg, sizeof msg, "Parede! Nao da para ir %s dali.", ch == 'w' ? "para cima" : "para a esquerda");
                continue;
            }
            long long pv = at->pv + c->valor[IDX(c, nl, nc)];
            caminho[n++] = (Passo){nl, nc, pv};
            visitado[IDX(c, nl, nc)] = 1;
            if (pv <= 0)
                snprintf(msg, sizeof msg, "*** Voce morreu! Use Z para desfazer ou R para reiniciar. ***");
            else if (nl == c->lf && nc == c->cf)
                fim = 1;
        } else if (ch == 'z') {
            if (n > 1) {
                visitado[IDX(c, caminho[n - 1].l, caminho[n - 1].c)] = 0;
                n--;
            } else 
                snprintf(msg, sizeof msg, "Nada para desfazer.");
        } else if (ch == 'r') {
            while (n > 1) {
                visitado[IDX(c, caminho[n - 1].l, caminho[n - 1].c)] = 0;
                n--;
            }
            snprintf(msg, sizeof msg, "Reiniciado.");
        } else if (ch == 'q') {
            sair = 1;
        } else if (ch == 's' || ch == 'd') {
            snprintf(msg, sizeof msg, "So e permitido ir para cima (W) ou para a esquerda (A).");
        } else if (t != 0) {
            snprintf(msg, sizeof msg, "Tecla desconhecida.");
        }
    }
    modoTeclado(0);

    if (fim) {
        Passo *at = &caminho[n - 1];
        desenharTela(c, visitado, at, "");
        printf("\n*** Voce escapou da caverna com %lld pontos de vida! ***\n", at->pv);

        long long otimo;
        if (vidaOtima(c, &otimo) == 1) {
            if (at->pv >= otimo) 
                printf("Voce fez o melhor caminho possivel (otimo da PD = %lld)!\n", otimo);
            else 
                printf("O otimo da PD e %lld PV; faltaram %lld. Tente de novo!\n", otimo, otimo - at->pv);
        }
        if (salvarCaminho(caminho, n, saida) == 0)
            printf("Seu caminho foi gravado em \"%s\".\n", saida);
    } else if (sair) {
        if (interativo()) 
            limparTela();
        printf("Saindo sem completar a caverna.\n");
    }

    free(visitado); 
    free(caminho);
    return 0;
}