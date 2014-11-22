defaulttarget: all

CFLAGS = -Wimplicit -Wstrict-prototypes -Wall -g
CC = gcc

connect.o: connect.c
	$(CC) $(CFLAGS) -c -o connect.o connect.c

connect: connect.o
	$(CC) -o connect connect.o

all: connect
clean:
	rm connect connect.o
