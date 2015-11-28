CC=gcc
CFLAGS=-Wall -O0

all: recovery

recovery: main.o list_directory.o
	gcc -o recovery $^

main.o: main.c list_directory.h
	gcc -c $< 

list_directory.o: list_directory.c list_directory.h data_struct.h
	gcc -c $<