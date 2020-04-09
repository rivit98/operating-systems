CC=gcc
CFLAGS=-Wall -Wno-unused-variable

all:
	$(CC) $(CFLAGS) producer.c -o producer
	$(CC) $(CFLAGS) consumer.c -o consumer
	$(CC) $(CFLAGS) runner.c -o runner

clean:
	rm -f consumer producer runner *.o

test: clean all
	./runner
