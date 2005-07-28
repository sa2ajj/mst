CC=gcc

CFLAGS=-Wall $(GTKFLAGS)
LDFLAGS=$(GTKLDFLAGS)

GTKFLAGS:=$(shell pkg-config --cflags gtk+-2.0)
GTKLDFLAGS:=$(shell pkg-config --libs gtk+-2.0)

all: mst

mst: mst.o http.o util.o screenshot.o

mst-s: mst.o http.o util.o screenshot.o
	$(CC) -o mst-s -static $(LDFLAGS) mst.o http.o util.o screenshot.o

mst.o: mst.c http.h util.h

http.o: http.c http.h util.h

util.o: util.c util.h

screenshot.o: screenshot.c screenshot.h

clean:
	rm -f mst *.o
