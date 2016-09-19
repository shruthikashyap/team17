CC=gcc
CFLAGS = -g -Wall -lm -pthread
EXEC = ./pc-terminal

all:
	$(CC) $(CFLAGS) *.c ../crc.c -o $(EXEC)

run: all
	$(EXEC)

