CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lm

all: c-wildwater converter

c-wildwater: main.o avl.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

converter: converter.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c avl.h structs.h
	$(CC) $(CFLAGS) -c main.c

avl.o: avl.c avl.h structs.h
	$(CC) $(CFLAGS) -c avl.c

converter.o: converter.c
	$(CC) $(CFLAGS) -c converter.c

clean:
	rm -f *.o *.tmp *~ c-wildwater converter

.PHONY: all clean