CC=gcc
CFLAGS=-Wall -Wno-unused-variable
LIB_NAME=diffLib
LIB_PATH=../zad1
REPORT_FILENAME=raport2.txt
REPORT_CMD=2>/dev/null >> $(REPORT_FILENAME)

static:
	make -C $(LIB_PATH) static

clean:
	make -C $(LIB_PATH) clean
	rm -f *.o main temp.txt $(REPORT_FILENAME)

valgrind: static
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./main `cat ./test_files/args_1.txt | tr '\r\n' ' '`

test: static
	$(CC) $(CFLAGS) main.c -I$(LIB_PATH) -l$(LIB_NAME) -L$(LIB_PATH) -D PERFORMANCE_TESTS -o main
	make report

report:
	rm -f $(REPORT_FILENAME)
	@printf "Mala ilosc plikow - bardzo podobne\n" $(REPORT_CMD)
	./main `cat ./test_files/args_1.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Mala ilosc plikow - srednio podobne\n" $(REPORT_CMD)
	./main `cat ./test_files/args_2.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Mala ilosc plikow - malo podobne\n" $(REPORT_CMD)
	./main `cat ./test_files/args_3.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Srednia ilosc plikow - bardzo podobne\n" $(REPORT_CMD)
	./main `cat ./test_files/args_4.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Srednia ilosc plikow - srednio podobne\n" $(REPORT_CMD)
	./main `cat ./test_files/args_5.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Srednia ilosc plikow - malo podobne\n" $(REPORT_CMD)
	./main `cat ./test_files/args_6.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Duza ilosc plikow - bardzo podobne\n" $(REPORT_CMD)
	./main `cat ./test_files/args_7.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Duza ilosc plikow - srednio podobne\n" $(REPORT_CMD)
	./main `cat ./test_files/args_8.txt | tr '\r\n' ' '` $(REPORT_CMD)

	@printf "Duza ilosc plikow - malo podobne\n" $(REPORT_CMD)
	./main `cat ./test_files/args_9.txt | tr '\r\n' ' '` $(REPORT_CMD)
