CC=gcc
LIBS=-lm
WARNINGS=-g
BINARIES=FBereken FMutatie FUitvoer
DESTDIR=/home/kai/foef/FOEFSys/bin

all:	$(BINARIES)

install:
	install -m 755 FBereken $(DESTDIR)/
	install -m 755 FMutatie $(DESTDIR)/
	install -m 755 FUitvoer $(DESTDIR)/

.c.o:
	$(CC) -c -std=gnu99 $(WARNINGS) $<


FBereken:	FBereken.o FBasis.o
	$(CC) -o $@ $^ $(LIBS)

FMutatie:	FMutatie.o FBasis.o
	$(CC) -o $@ $^ $(LIBS)

FUitvoer:	FUitvoer.o FBasis.o
	$(CC) -o $@ $^ $(LIBS)

#gcc   FBereken.o FBasis.o   -o FBereken

#$(CC) -o $@ $(LIBS)
#$(CC) -o $@ $(LIBS)
#$(CC) -o $@ $(LIBS)


clean: 
	rm -f $(BINARIES) *.o *.h.gch
