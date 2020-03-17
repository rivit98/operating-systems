CC=gcc
CFLAGS=-Wall -Wno-unused-variable

all: find.o find_nftw.o find_lib.o
	$(CC) $(CFLAGS) find.o find_lib.o -o find
	$(CC) $(CFLAGS) find_nftw.o find_lib.o -o find_nftw

%.o : %.c find_lib.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f find find_nftw *.o

test: clean all
	printf "\n"
	./find . | wc -l
	./find_nftw . | wc -l
	./find . -maxdepth 0 | wc -l
	./find_nftw . -maxdepth 0 | wc -l
	./find . -maxdepth 1 | wc -l
	./find_nftw . -maxdepth 1 | wc -l
	./find . -maxdepth 2 | wc -l
	./find_nftw . -maxdepth 2 | wc -l
	./find . -maxdepth 3 | wc -l
	./find_nftw . -maxdepth 3 | wc -l
	./find . -maxdepth 4 | wc -l
	./find_nftw . -maxdepth 4 | wc -l