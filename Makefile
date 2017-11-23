#makefile for sf2float
OBJS = sf2float.o
EXE = sf2float
CC = gcc
CFLAGS = -c -I ./include
LFLAGS = -lportsf -lm #libportsf.a in usr/local/lib

build:	$(OBJS)
	$(CC) $(OBJS) -o $(EXE) $(LFLAGS)

sf2float.o:	sf2float.c
	$(CC) $(CFLAGS) sf2float.c

clean:
	-rm -f $(EXE) $(OBJS)

rebuild: clean build
