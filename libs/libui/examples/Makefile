CC = gcc
CFLAGS  := -g -Os -Wall -I/usr/local/include
#LDFLAGS := -L.. -L/usr/local/lib -lSDL3 -lSDL3_ttf -lm -l:libui.a
LDFLAGS := -L.. -L/usr/local/lib -lSDL3 -lSDL3_ttf -lm 

showfont: showfont.o editbox.o
	$(CC) showfont.o editbox.o $(LDFLAGS) -o showfont

.PHONY: all
all: showfont

.PHONY: clean
clean:
	rm -f *.o showfont 

.PHONY: deps
deps:
	$(CC) $(CFLAGS) -MM *.c > Makefile.deps

include Makefile.deps

