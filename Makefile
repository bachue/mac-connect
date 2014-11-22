defaulttarget: connect

CFLAGS = -Wimplicit -Wstrict-prototypes -Wall -g
CC = gcc

connect.o: connect.c
	$(CC) $(CFLAGS) -c -o connect.o connect.c

config.o: config.c
	$(CC) $(CFLAGS) -c -o config.o config.c

common.o: common.c
	$(CC) $(CFLAGS) -c -o common.o common.c

connect: connect.o common.o config.o
	$(CC) -o connect common.o config.o connect.o

test_config: config.c common.o
	$(CC) -DTEST -g -o test_config common.o config.c

clean:
	rm connect *.o
