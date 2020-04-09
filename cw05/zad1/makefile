CC=gcc
CFLAGS=-Wall -Wno-unused-variable

all:
	$(CC) $(CFLAGS) main.c -o main

clean:
	rm -f main *.o

test: all
	./main params.txt

valgrid: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./main params.txt
