INCLUDES = -I/usr/local/include -I./include
# -Ldir adds a directory to the search path [dir]
# -lxxx searches for a library named libxxx
LIBS = -L/usr/local/lib -L./lib -lportaudio -lportsf -ltiny -lcrack
WAR = -Wall -Wextra
CC = gcc

all: hellotable player

helloring:
	$(CC) -o bin/helloring src/helloring.c $(INCLUDES) $(LIBS)

hellotable:	
	$(CC) -o bin/hellotable src/hellotable.c $(INCLUDES) $(LIBS) 

player:
	$(CC) -o bin/player src/player.c $(INCLUDES) $(LIBS)

player2:
	$(CC) $(WAR) -o bin/player2 src/player2.c $(INCLUDES) $(LIBS)

clean:
	-rm -f bin/hellotable
