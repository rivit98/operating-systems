CC=gcc
CFLAGS=-Wall -Wno-unused-variable

all:
	$(CC) $(CFLAGS) server.c -o server -lrt
	$(CC) $(CFLAGS) client.c -o client -lrt

clean:
	rm -f client server *.o
