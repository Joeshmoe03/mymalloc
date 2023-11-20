CFLAGS=-Wall -g -pedantic

PROGS=myio

mymalloc: my-malloc.c
	gcc $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o $(PROGS)
