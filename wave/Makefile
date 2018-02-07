# makefile for sfenv
CC = gcc
EXE = siggen
OBJS = $(EXE).o wave.o breakpoints.o
CFILES = $(EXE).c wave.c breakpoints.c
CFLAGS = -c -Wall -I ../include
LFLAGS = -lportsf -lm

build: $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(EXE) $(LFLAGS)

$(EXE).o: $(CFILES)
	$(CC) $(CFLAGS) $(EXE).c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
