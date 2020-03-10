CC=gcc
CFLAGS=-Wall -Wno-unused-variable
LIB_NAME=diffLib
LIB_PATH=../zad1
MAIN_PROG_LOCATION=../zad2
MAIN_PROG_FILE_C=main.c
MAIN_PROG_PATH=$(MAIN_PROG_LOCATION)/$(MAIN_PROG_FILE_C)
REPORT_FILENAME=raport3b.txt
REPORT_CMD=2>/dev/null >> $(REPORT_FILENAME) 

static:
	make -C $(LIB_PATH) static
	$(CC) $(CFLAGS) $(OPTIMIZATION) $(MAIN_PROG_PATH) -I$(LIB_PATH) -l$(LIB_NAME) -L$(LIB_PATH) -D PERFORMANCE_TESTS -o main

shared:
	make -C $(LIB_PATH) shared
	$(CC) $(CFLAGS) $(OPTIMIZATION) $(MAIN_PROG_PATH) -I$(LIB_PATH) -l$(LIB_NAME) -L$(LIB_PATH) -D PERFORMANCE_TESTS -o main

dynamic:
	make -C $(LIB_PATH) shared
	$(CC) $(CFLAGS) $(OPTIMIZATION) $(MAIN_PROG_PATH) -ldl -D LIB_DYNAMIC -D PERFORMANCE_TESTS -o main

clean:
	make -C $(LIB_PATH) clean
	make -C $(MAIN_PROG_LOCATION) clean
	rm -f $(REPORT_FILENAME) temp.txt main

test: clean
	# rather bad solution, but...
	cp -r $(MAIN_PROG_LOCATION)/test_files/ ./

	for lib in static shared dynamic ; do \
		for o in O0 O1 O2 O3 Ofast Os ; do \
			make $$lib OPTIMIZATION=-$$o; \
			printf "$$lib $$o\n" $(REPORT_CMD) ; \
			make report ; \
		done ; \
	done

	rm -rf test_files

report:
	@printf "Mala ilosc plikow - bardzo podobne\n" $(REPORT_CMD)
	./main `cat $(MAIN_PROG_LOCATION)/test_files/args_1.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Mala ilosc plikow - srednio podobne\n" $(REPORT_CMD)
	./main `cat $(MAIN_PROG_LOCATION)/test_files/args_2.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Mala ilosc plikow - malo podobne\n" $(REPORT_CMD)
	./main `cat $(MAIN_PROG_LOCATION)/test_files/args_3.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Srednia ilosc plikow - bardzo podobne\n" $(REPORT_CMD)
	./main `cat $(MAIN_PROG_LOCATION)/test_files/args_4.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Srednia ilosc plikow - srednio podobne\n" $(REPORT_CMD)
	./main `cat $(MAIN_PROG_LOCATION)/test_files/args_5.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Srednia ilosc plikow - malo podobne\n" $(REPORT_CMD)
	./main `cat $(MAIN_PROG_LOCATION)/test_files/args_6.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Duza ilosc plikow - bardzo podobne\n" $(REPORT_CMD)
	./main `cat $(MAIN_PROG_LOCATION)/test_files/args_7.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Duza ilosc plikow - srednio podobne\n" $(REPORT_CMD)
	./main `cat $(MAIN_PROG_LOCATION)/test_files/args_8.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Duza ilosc plikow - malo podobne\n" $(REPORT_CMD)
	./main `cat $(MAIN_PROG_LOCATION)/test_files/args_9.txt | tr '\r\n' ' '` $(REPORT_CMD)