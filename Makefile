CFLAGS=-Wall -g -pedantic -rdynamic -shared -fPIC

PROGS=my-malloc.so

my-malloc.so: my-malloc.c
	gcc $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o $(PROGS)
