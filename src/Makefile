CC      = gcc
# Use compiler optimizations to speed up execution.
# The -O2 option enables general optimizations and -march=native allows
# using host architecture instructions. The -g flag is deliberately omitted
# to avoid disabling certain optimizations.
CFLAGS  = -Wall -Wextra -std=c99 -O2 -march=native
LDFLAGS = -lm

# List of project source files. If you add other modules, simply
# add the .c files to this variable so they are compiled automatically.
SRCS    = main.c avl.c
OBJS    = $(addprefix bin/,$(SRCS:.c=.o))
TARGET  = bin/c-wildwater

all: bin $(TARGET)

bin:
	mkdir -p bin

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compilation rules for object files
bin/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rmdir bin 2>/dev/null || true

.PHONY: all clean