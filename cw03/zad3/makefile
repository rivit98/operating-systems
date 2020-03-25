CC=gcc
CFLAGS=-Wall -Wno-unused-variable

all:
	$(CC) $(CFLAGS) macierz.c -o macierz
	$(CC) $(CFLAGS) tester.c -o tester
	$(CC) $(CFLAGS) generator.c -o generator

clean:
	rm -rf macierz tester generator lista.txt *.o ./tmp ./out ./data

test: clean all
	@echo ""; echo Generuje macierze...
	./generator 1 100 100

	@echo ""; echo Mnoze macierze...
	./macierz lista.txt 10 5 2 2 1

	@echo ""; echo Sprawdzam poprawnosc...
	./tester lista.txt