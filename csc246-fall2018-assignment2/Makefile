CC = gcc
CFLAGS = -Wall -D_XOPEN_SOURCE=500 -std=c99 -g

all: reset fifteen
reset: reset.o
fifteen: fifteen.o
reset.o: reset.c
fifteen.o: fifteen.c common.h

clean:
	rm -f *.o
