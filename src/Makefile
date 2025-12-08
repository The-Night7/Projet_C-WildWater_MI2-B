CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lm

all: c-wildwater

c-wildwater: main.o avl.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.c avl.h structs.h
	$(CC) $(CFLAGS) -c main.c

avl.o: avl.c avl.h structs.h
	$(CC) $(CFLAGS) -c avl.c

clean:
	rm -f *.o c-wildwater

.PHONY: all clean