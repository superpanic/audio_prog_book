# makefile for sfenv
CC = gcc
EXE = siggen
OBJS = $(EXE).o wave.o
CFILES = $(EXE).c wave.c
CFLAGS = -c -I ../include
LFLAGS = -lportsf -lm

build: $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(EXE) $(LFLAGS)

sfpan.o: $(CFILES)
	$(CC) $(CFLAGS) $(EXE).c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
