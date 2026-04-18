all: gobbler.prg

gobbler.s: gobbler.c
	cc65 -t c64 gobbler.c -o gobbler.s

gobbler.o: gobbler.s
	ca65 -t c64 gobbler.s -o gobbler.o

gobbler.prg: gobbler.o
	cl65 -t c64 gobbler.o -o gobbler.prg

.PHONY: clean

clean:
	rm gobbler.prg gobbler.o gobbler.s

