netINC= -I/usr/include
LIB= -L/usr/lib
CFLAGS=-Wall -g ${INC} -L. ${LIB}
CC=gcc

ep1.o: ep1.c
	$(CC) $(CFLAGS) -c $*.c

ep1 : ep1.o
	${CC} ${CFLAGS} -o ep1 ep1.o

clean: 
	rm -f *.o ep1 *~