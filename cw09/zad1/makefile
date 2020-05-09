CC=gcc
CFLAGS=-Wall -Wno-unused-variable
LIBS=-lpthread

all:
	$(CC) $(CFLAGS) main.c -o main $(LIBS)

clean:
	rm -f main *.o

test: clean all
	./main 2 15