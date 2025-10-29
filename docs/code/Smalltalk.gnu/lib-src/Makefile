all: library.a

OBJS = acosl.o alloca.o asinl.o atanl.o avltrees.o ceill.o cosl.o expl.o \
	floorl.o frexpl.o getaddrinfo.o getdtablesize.o getopt1.o getopt.o \
	getpagesize.o inet_ntop.o ldexpl.o lock.o logl.o lrint.o lrintf.o lrintl.o \
	ltdl.o md5.o mkdtemp.o mkstemp.o obstack.o pointer-set.o poll.o powl.o \
	putenv.o rbtrees.o regex.o setenv.o sincosl.o sinl.o socketx.o sqrtl.o \
	strdup.o strerror.o strpbrk.o strsep.o strsignal.o strspell.o strstr.o \
	strtoul.o symlink.o tanl.o trigl.o trunc.o truncf.o truncl.o


library.a: $(OBJS)
	ar rcs library.a $(OBJS)

clean:
	rm -f $(OBJS)

acosl.o: acosl.c mathl.h
alloca.o: alloca.c
asinl.o: asinl.c mathl.h
atanl.o: atanl.c mathl.h
avltrees.o: avltrees.c avltrees.h
ceill.o: ceill.c mathl.h
cosl.o: cosl.c mathl.h trigl.h
expl.o: expl.c mathl.h
floorl.o: floorl.c mathl.h
frexpl.o: frexpl.c mathl.h
getaddrinfo.o: getaddrinfo.c socketx.h
getdtablesize.o: getdtablesize.c
getopt1.o: getopt1.c getopt.h
getopt.o: getopt.c
getpagesize.o: getpagesize.c
inet_ntop.o: inet_ntop.c socketx.h
ldexpl.o: ldexpl.c mathl.h
lock.o: lock.c lock.h
logl.o: logl.c mathl.h
lrint.o: lrint.c mathl.h
lrintf.o: lrintf.c mathl.h
lrintl.o: lrintl.c mathl.h
ltdl.o: ltdl.c
md5.o: md5.c md5.h
mkdtemp.o: mkdtemp.c
mkstemp.o: mkstemp.c
obstack.o: obstack.c obstack.h
pointer-set.o: pointer-set.c pointer-set.h
poll.o: poll.c
powl.o: powl.c mathl.h
putenv.o: putenv.c ../libgst/alloc.h
rbtrees.o: rbtrees.c rbtrees.h
regex.o: regex.c regex.h
setenv.o: setenv.c
sincosl.o: sincosl.c mathl.h
sinl.o: sinl.c mathl.h trigl.h trigl.c sincosl.c
socketx.o: socketx.c
sqrtl.o: sqrtl.c mathl.h
strdup.o: strdup.c ../libgst/alloc.h
strerror.o: strerror.c
strpbrk.o: strpbrk.c
strsep.o: strsep.c
strsignal.o: strsignal.c ansidecl.h
strspell.o: strspell.c strspell.h
strstr.o: strstr.c
strtoul.o: strtoul.c
symlink.o: symlink.c
tanl.o: tanl.c mathl.h trigl.h
trigl.o: trigl.c mathl.h
trunc.o: trunc.c mathl.h
truncf.o: truncf.c mathl.h
truncl.o: truncl.c mathl.h

