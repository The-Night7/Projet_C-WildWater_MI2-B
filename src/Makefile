# Makefile for C-WildWater project

# Compiler configuration
CC      = gcc
CFLAGS  = -Wall -O3 -march=native -flto -ffast-math -Wextra -std=c99 -pthread
LDFLAGS = -lm -pthread

# Source files
SRCS    = main.c avl.c
OBJS    = $(addprefix bin/,$(SRCS:.c=.o))

# Main executable
TARGET  = bin/c-wildwater

# Default target
all: bin $(TARGET)

# Create bin directory if it doesn't exist
bin:
	mkdir -p bin

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files to object files
bin/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)
	rmdir bin 2>/dev/null || true

.PHONY: all clean