CC=gcc
CFLAGS=-Wall -Wno-unused-variable

all:
	$(CC) $(CFLAGS) server.c -o server
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm -f client server *.o
