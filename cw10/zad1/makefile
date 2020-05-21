CC=gcc
CFLAGS=-Wall -Wno-unused-variable
LIBS=-lpthread

all:
	$(CC) $(CFLAGS) server.c -o server $(LIBS)
	$(CC) $(CFLAGS) client.c -o client $(LIBS)

clean:
	rm -f client server *.o