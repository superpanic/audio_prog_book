# makefile for sfenv
CC = gcc
EXE = oscgen
OBJS = $(EXE).o wave.o breakpoints.o
IPATH = ../include/
CFILES = $(EXE).c $(IPATH)wave.c $(IPATH)breakpoints.c
CFLAGS = -c -Wall -I ../include
LFLAGS = -lportsf -lm

build: $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(EXE) $(LFLAGS)

oscgen.o: $(CFILES)
	$(CC) $(CFLAGS) $(EXE).c

wave.o: $(CFILES)
	$(CC) $(CFLAGS) $(IPATH)wave.c

breakpoints.o: $(CFILES)
	$(CC) $(CFLAGS) $(IPATH)breakpoints.c

clean:
	-rm -f $(OBJS) $(EXE)

rebuild: clean build
