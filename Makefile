CC=gcc
CCFLAGS=-I include -std=c11 -Wall
HEADERS=$(wildcard include/*.h)
LIBFILES=$(wildcard lib/*.c)
SRCFILES=$(wildcard src/*.c)

bin/spark: $(HEADERS) $(LIBFILES) $(SRCFILES)
	$(CC) $(CCFLAGS) $(LIBFILES) $(SRCFILES) -o bin/spark



.PHONY: clean
clean:
	rm -f bin/*
