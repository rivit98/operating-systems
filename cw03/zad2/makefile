CC=gcc
CFLAGS=-Wall -Wno-unused-variable -ggdb

all:
	$(CC) $(CFLAGS) macierz.c -o macierz
	$(CC) $(CFLAGS) tester.c -o tester
	$(CC) $(CFLAGS) generator.c -o generator

clean:
	rm -rf macierz tester generator lista.txt *.o ./tmp ./out ./data

test: clean all
	@echo ""; echo Generuje macierze...
	./generator 1 200 100

	@echo ""; echo Mnoze macierze...
	./macierz lista.txt 10 50 2

	@echo ""; echo Sprawdzam poprawnosc...
	./tester lista.txt

valgrind: clean all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./generator 1 20 50
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./macierz lista.txt 3 120 1
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./tester lista.txt