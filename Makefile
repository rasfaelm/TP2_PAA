CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -D__USE_MINGW_ANSI_STDIO=1

# Detecta o sistema operacional
ifeq ($(OS),Windows_NT)
    CFLAGS += -D__USE_MINGW_ANSI_STDIO=1
    EXEC_EXT = .exe
    RM = del /Q
    RMDIR = rmdir /S /Q
else
    EXEC_EXT =
    RM = rm -f
    RMDIR = rm -rf
endif

SRC_COMUM = caverna.c mapa_manual.c jogo_manual.c guloso.c GeradorDeMapa.c arte.c
OBJ_COMUM = $(SRC_COMUM:.c=.o)

all: caverna$(EXEC_EXT) bench$(EXEC_EXT)

caverna$(EXEC_EXT): main.o $(OBJ_COMUM)
	$(CC) $(CFLAGS) -o $@ $^

bench$(EXEC_EXT): bench.o caverna.o guloso.o arte.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c caverna.h mapa_manual.h jogo_manual.h guloso.h GeradorDeMapa.h arte.h
bench.o: bench.c caverna.h guloso.h
caverna.o: caverna.c caverna.h
mapa_manual.o: mapa_manual.c mapa_manual.h caverna.h
jogo_manual.o: jogo_manual.c jogo_manual.h caverna.h arte.h
guloso.o: guloso.c guloso.h caverna.h arte.h
GeradorDeMapa.o: GeradorDeMapa.c GeradorDeMapa.h caverna.h
arte.o: arte.c arte.h

# make run  ->  abre o menu interativo
run: caverna$(EXEC_EXT)
	./caverna$(EXEC_EXT)

# make run-mapa MAPA=testes/caverna1.txt  ->  resolve um mapa direto, sem menu
MAPA ?= testes/caverna1.txt

run-mapa: caverna$(EXEC_EXT)
	./caverna$(EXEC_EXT) "$(MAPA)"
	python3 -c "print(open('resultado.txt').read())"

# make grafico
grafico: bench$(EXEC_EXT)
	./bench$(EXEC_EXT) 2000 200 5 bench.csv
	python3 plot.py bench.csv grafico.png

# make comparar
comparar: caverna$(EXEC_EXT)
	python3 comparar.py

clean:
ifeq ($(OS),Windows_NT)
	-$(RM) *.o caverna.exe bench.exe
	-$(RM) caminho_manual.txt resultado_otimo.tmp resultado_guloso.txt resultado_pd.tmp
	-$(RM) resultado.txt bench_saida.tmp bench.csv grafico.png
	-$(RMDIR) comparacao
else
	$(RM) *.o caverna bench
	$(RM) caminho_manual.txt resultado_otimo.tmp resultado_guloso.txt resultado_pd.tmp
	$(RM) resultado.txt bench_saida.tmp bench.csv grafico.png
	$(RMDIR) comparacao
endif

.PHONY: all run run-mapa grafico comparar clean