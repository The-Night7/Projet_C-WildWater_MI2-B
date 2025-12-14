CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -g
LDFLAGS = -lm

# Liste des sources du projet.  Si vous ajoutez d’autres modules, ajoutez
# simplement les fichiers .c dans cette variable pour qu’ils soient compilés
# automatiquement.
SRCS    = main.c avl.c utils.c
OBJS    = $(addprefix bin/,$(SRCS:.c=.o))
TARGET  = bin/c-wildwater

all: bin $(TARGET)

bin:
	mkdir -p bin

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Règles de compilation des fichiers objets
bin/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rmdir bin 2>/dev/null || true

.PHONY: all clean
