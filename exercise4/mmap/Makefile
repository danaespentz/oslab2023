.PHONY: all clean

all: mmap

CC = gcc
CFLAGS = -g -Wall -Wextra -O2

mmap: mmap.o help.o
		$(CC) $(CFLAGS) -o mmap mmap.o help.o

mmap.o: mmap.c help.h
		$(CC) $(CFLAGS) -c mmap.c

clean:
		rm -f mmap.o mmap