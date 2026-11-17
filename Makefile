CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2

SRC_COMUM = caverna.c mapa_manual.c
OBJ_COMUM = $(SRC_COMUM:.c=.o)

all: caverna bench

caverna: main.o $(OBJ_COMUM)
	$(CC) $(CFLAGS) -o $@ $^

bench: bench.o caverna.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c caverna.h mapa_manual.h
bench.o: bench.c caverna.h
caverna.o: caverna.c caverna.h
mapa_manual.o: mapa_manual.c mapa_manual.h caverna.h

# make run MAPA=testes/caverna1.txt
MAPA ?= testes/caverna1.txt
run: caverna
	./caverna $(MAPA)
	@cat resultado.txt

# make grafico -> roda o benchmark e plota (precisa de python3 + matplotlib)
grafico: bench
	./bench 2000 200 5 bench.csv
	python3 plot.py bench.csv grafico.png

clean:
	rm -f *.o caverna bench caverna.exe bench.exe
	rm -f resultado.txt bench_saida.tmp bench.csv grafico.png

.PHONY: all run grafico clean