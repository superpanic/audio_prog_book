#makefile for sf2float
CC = gcc
EXE = sf2float
CFILES = $(EXE).c
OBJS = $(EXE).o
CFLAGS = -c -I ../include
LFLAGS = -lportsf -lm #libportsf.a in usr/local/lib

build:	$(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

sf2float.o:	$(CFILES)
	$(CC) $(CFLAGS) $(CFILES)

clean:
	-rm -f $(EXE) $(OBJS)

rebuild: clean build
