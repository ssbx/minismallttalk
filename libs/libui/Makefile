CC = gcc
CFLAGS  := -g -Os -Wall -I. -I/usr/local/include
#LIB_LDFLAGS := -L. -L/usr/local/lib -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL3_ttf -lSDL3 -l:libui.a
LDFLAGS := -L/usr/local/lib -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL3_ttf -lSDL3 

OBJS := frame.o layout.o context.o theme.o editbox.o window.o

.PHONY: run
run: demo2
	./demo2

#libui.a: $(LIBOBJS)
	#ar rcs libui.a $(LIBOBJS)

.PHONY: all
all: demo demo2
	make -C examples all

demo: demo.o $(OBJS)
	$(CC) demo.o $(OBJS) $(LDFLAGS) -o demo

demo2: demo2.o $(OBJS)
	$(CC) demo2.o $(OBJS) $(LDFLAGS) -o demo2


.PHONY: example
example: examples/showfont
	 ./examples/showfont fonts/DejaVuSans.ttf

.PHONY:
examples/showfont: libui.a
	make -C examples showfont

.PHONY: clean
clean:
	rm -f *.o libui.a demo demo2
	make -C examples clean

.PHONY: deps
deps:
	$(CC) $(CFLAGS) -MM *.c > Makefile.deps

include Makefile.deps

