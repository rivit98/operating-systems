CC=gcc
CFLAGS=-Wall -Wno-unused-variable
RESULTS=wyniki.txt
REPORT_CMD=2>/dev/null >> $(RESULTS) 

all: main.o io_lib.o
	$(CC) $(CFLAGS) main.o io_lib.o -o main

%.o : %.c io_lib.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main *.o
	rm -rf ./data

test: clean all

	rm $(RESULTS)
	mkdir ./data

	for records_num in 500 3000 ; do \
		for record_size in 1 4 512 1024 4096 8192 ; do \
			printf "Generating $$records_num records ($$record_size byte each)\n" $(REPORT_CMD); \
			./main generate ./data/dane-lib-$$records_num-$$record_size.txt $$records_num $$record_size $(REPORT_CMD) ;\
			cp ./data/dane-lib-$$records_num-$$record_size.txt ./data/dane-sys-$$records_num-$$record_size.txt ;\
			printf "\n" $(REPORT_CMD) ; \
			for method in lib sys ; do \
				printf "Sorting $$records_num ($$record_size byte) $$method\n" $(REPORT_CMD) ; \
				./main sort ./data/dane-$$method-$$records_num-$$record_size.txt $$records_num $$record_size $$method $(REPORT_CMD) ;\
				printf "\n" $(REPORT_CMD) ; \
				printf "." ;\
			done ; \
			for method in lib sys ; do \
				printf "Copying $$records_num ($$record_size byte) $$method\n" $(REPORT_CMD) ; \
				./main copy ./data/dane-$$method-$$records_num-$$record_size.txt ./data/dane-$$method-$$records_num-$$record_size-copy.txt $$records_num $$record_size $$method $(REPORT_CMD) ;\
				printf "\n" $(REPORT_CMD) ; \
			done ; \
			printf "\n" $(REPORT_CMD) ; \
		done ; \
	done ;\
	printf "\n"