CC=gcc
CFLAGS=-Wall
LIB_NAME=diffLib

static:
	$(CC) -c $(CFLAGS) $(LIB_NAME).c -o $(LIB_NAME).o
	ar rcs lib$(LIB_NAME).a $(LIB_NAME).o

shared:
	$(CC) -fPIC -c $(CFLAGS) $(LIB_NAME).c -o $(LIB_NAME).o
	$(CC) -fPIC -shared $(CFLAGS) $(LIB_NAME).o -o $(LIB_NAME).so

clean:
	rm -f *.o *.a *.so