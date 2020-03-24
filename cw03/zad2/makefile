CC=gcc
CFLAGS=-Wall -Wno-unused-variable -ggdb

all:
	$(CC) $(CFLAGS) macierz.c -o macierz

clean:
	rm -rf macierz *.o ./tmp ./out

test: clean all
	./macierz lista.txt 2 5

valgrind: clean all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./macierz lista.txt 2 5