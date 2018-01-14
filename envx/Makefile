# makefile for envx
CC = gcc
EXE = envx
OBJS = $(EXE).o
CFILES = $(EXE).c
CFLAGS = -c -I ../include
LFLAGS = -lportsf -lm

build: $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(EXE) $(LFLAGS)

sfpan.o: $(CFILES)
	$(CC) $(CFLAGS) $(EXE).c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
