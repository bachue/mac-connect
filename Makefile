defaulttarget: connect

.PHONY: clean
.PHONY: install
.PHONY: test_config
.PHONY: release

CC = gcc
CFLAGS = -Wimplicit -Wstrict-prototypes -Wall

INSTALL_ROOT ?= /usr/bin

ifeq ($(RELEASE), 1)
    CFLAGS += -O3
else
    CFLAGS += -g
endif

connect.o: connect.c
	$(CC) $(CFLAGS) -c -o connect.o connect.c

config.o: config.c
	$(CC) $(CFLAGS) -c -o config.o config.c

common.o: common.c
	$(CC) $(CFLAGS) -c -o common.o common.c

connect: connect.o common.o config.o
	$(CC) -o connect common.o config.o connect.o

install: clean
	make RELEASE=1 && \
	cp connect $(INSTALL_ROOT)

release: clean
	make RELEASE=1 && \
	tar -jcvf connect.tar.bz2 connect contrib && \
	shasum connect.tar.bz2

test_config: config.c common.o
	$(CC) -DTEST -g -o test_config common.o config.c

clean:
	rm -f connect *.o *.tar.bz2
