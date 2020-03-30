CC=gcc
CFLAGS=-Wall -Wno-unused-variable


all: 
	$(CC) $(CFLAGS) sender.c -o sender
	$(CC) $(CFLAGS) catcher.c -o catcher

clean:
	if pgrep catcher; then pkill --signal 9 catcher; fi
	if pgrep sender; then pkill --signal 9 sender; fi
	rm -f catcher sender *.o