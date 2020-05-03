CC=gcc
CFLAGS=-Wall -Wno-unused-variable
LIBS=-lpthread -lm
RESULTS=Times.txt
REPORT_CMD=2>/dev/null >> $(RESULTS)

all:
	$(CC) $(CFLAGS) main.c -o main $(LIBS)

clean:
	rm -f main *.o $(RESULTS)

test: clean all
	for thread_num in 1 2 4 8; do \
		for method in sign block interleaved ; do \
			printf "Processing file with $$thread_num threads using $$method\n" $(REPORT_CMD) ; \
			rm -rf ./out_$$method $(REPORT_CMD) ; \
			./main $$thread_num $$method ./data/venus1.ascii.pgm ./out_$$method $(REPORT_CMD) ; \
			printf "\n\n" $(REPORT_CMD) ; \
			rm -rf ./out_$$method $(REPORT_CMD) ; \
		done ; \
	done ;\
	printf "\nDone!\n"