all: zing zing2

zing: main.o zing.o
                gcc -o zing main.o zing.o

zing2: zing2.o main.o
                gcc -o zing2 main.o zing2.o

zing2.o: zing2.c
                gcc -Wall -c zing2.c

main.o: main.c zing.h
                gcc -Wall -c main.c

clean:
            rm -f main.o zing zing2 zing2.o
