CC = gcc
CFLAGS = -Wall -O2

.PHONY: mandel-fork mandel-fork-sem  clean

mandel-fork: mandel-fork.o ../helpers/mandel-lib.o
	$(CC) $(CFLAGS) -o mandel-fork mandel-fork.o ../helpers/mandel-lib.o -lm

mandel-fork-sem: mandel-fork-sem.o ../helpers/mandel-lib.o
	$(CC) $(CFLAGS) -o mandel-fork-sem mandel-fork-sem.o ../helpers/mandel-lib.o -lm

mandel-fork.o: mandel-fork.c ../helpers/mandel-lib.h
	$(CC) $(CFLAGS) -c mandel-fork.c

mandel-fork-sem.o: mandel-fork-sem.c ../helpers/mandel-lib.h
	$(CC) $(CFLAGS) -c mandel-fork-sem.c

clean:
	rm -f mandel-fork.o mandel-fork-sem.o mandel-fork mandel-fork-sem
