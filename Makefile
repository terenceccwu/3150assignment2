CC=gcc
CFLAGS=-Wall -O0

all: main.o read_device.o
	gcc -o recovery main.o read_device.o

main.o: main.c
	gcc -c main.c

read_device.o: read_device.c
	gcc -c read_device.c
