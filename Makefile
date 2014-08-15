CC=gcc
#CCOPTS=-g -Wall
CCOPTS=-O2
LIBS=-lm

all: spong

spong: spong.c spong.h
	$(CC) $(CCOPTS) -o spong spong.c \
		`/usr/bin/sdl-config --cflags --libs` $(LIBS)

fullscreen: spong.c spong.h
	$(CC) $(CCOPTS) -o spong spong.c -DFULL_SCREEN \
		`/usr/bin/sdl-config --cflags --libs` $(LIBS)

debug: spong.c spong.h
	$(CC) $(CCOPTS) -o spong spong.c -DPONG_DBUG \
		`/usr/bin/sdl-config --cflags --libs` $(LIBS)

clean:
	rm -f core spong


