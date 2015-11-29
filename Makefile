CC=gcc
CFLAGS=-Wall -O0

all: recovery

recovery: main.o list_directory.o recover.o
	gcc -o recovery $^

main.o: main.c list_directory.h recover.h
	gcc -c $< 

list_directory.o: list_directory.c list_directory.h data_struct.h
	gcc -c $<

recover.o: recover.c recover.h
	gcc -c $<