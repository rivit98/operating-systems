CC=gcc
CFLAGS=-Wall -Wno-unused-variable

all:
	$(CC) $(CFLAGS) find.c find_lib.c -o find
	$(CC) $(CFLAGS) -D USE_NFTW find.c find_lib.c -o find_nftw

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