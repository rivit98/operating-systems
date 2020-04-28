CC=gcc
CFLAGS=-Wall -Wno-unused-variable
LIBS=-lm

all:
	$(CC) $(CFLAGS) starter.c -o starter $(LIBS)
	$(CC) $(CFLAGS) worker1.c -o worker1 $(LIBS)
	$(CC) $(CFLAGS) worker2.c -o worker2 $(LIBS)
	$(CC) $(CFLAGS) worker3.c -o worker3 $(LIBS)

clean:
	rm -f starter worker1 worker2 worker3 *.o
