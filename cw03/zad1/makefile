CC=gcc
CFLAGS=-Wall -Wno-unused-variable

all:
	$(CC) $(CFLAGS) find.c -o find

clean:
	rm -f find *.o

test: clean all
	./find .
	./find . -maxdepth 1
	./find . -maxdepth 2
	./find . -maxdepth 3