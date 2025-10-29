
ifneq (,)
This makefile requires GNU Make.
endif

CC      ?= gcc
#CFLAGS  ?= -g -Os -ansi -Wall -I.
CFLAGS  ?= -g -Os -Wall -I.
CFLAGS  += $(shell pkg-config --cflags sdl3 ncurses)
LDFLAGS += $(shell pkg-config --libs sdl3 ncurses)
PROG = minist8
OBJS = bitblt.o system.o interpreter.o main.o objectmemory.o primitives.o \
       runtime.o

all: play

play: $(PROG)
	./$(PROG) -c 5000 -m -s -i images/snapshot4.im -o "--a --b asqdf"

run: $(PROG)
	./$(PROG) -v -c 5000 -m -s -i images/snapshot.im

clean:
	$(RM) -f *.o  *.st $(PROG) *.c~ *.h~ .depend images/*.st images/imageswapper

depend: .depend

%.st: images/%.st
	cat $< | tr "\015" "\n" > $@

.depend:
	$(CC) $(CFLAGS) -MM *.c > .depend

indent:
	indent *.c *.h

$(PROG): .depend $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(PROG)


-include .depend

.PHONY: all play run clean depend
