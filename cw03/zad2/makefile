CC=gcc
CFLAGS=-Wall -Wno-unused-variable

all:
	$(CC) $(CFLAGS) macierz.c -o macierz

clean:
	rm -f macierz *.o

test: all
	./macierz lista.txt 5 5