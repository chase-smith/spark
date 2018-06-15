CC=gcc
CCFLAGS=-I include -std=c11 -Wall
HEADERS=$(wildcard include/*.h)
LIBFILES=$(wildcard lib/*.c)
SRCFILES=$(wildcard src/*.c)

bin/spark: $(HEADERS) $(LIBFILES) $(SRCFILES)
	$(CC) $(CCFLAGS) $(LIBFILES) $(SRCFILES) -o bin/spark

SPARKDEMO.build: example/posts/*/* example/misc_pages/*/* example/series/*/* example/components/* example/themes/*/* bin/spark
	bin/spark --config SPARKDEMO.conf --generate-site | tee SPARKDEMO.build

.PHONY: clean
clean:
	rm -f bin/*
