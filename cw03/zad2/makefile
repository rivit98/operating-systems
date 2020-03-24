CC=gcc
CFLAGS=-Wall -Wno-unused-variable -ggdb

all:
	$(CC) $(CFLAGS) macierz.c -o macierz

clean:
	rm -rf macierz *.o ./tmp

test: clean all
	./macierz lista.txt 5 5