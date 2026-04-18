all: gobbler

gobbler: gobbler.c
	gcc gobbler.c -o gobbler

.PHONY: clean

clean:
	rm gobbler
