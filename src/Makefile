CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -g
LDFLAGS = -lm

# Liste des sources du projet.  Si vous ajoutez d’autres modules, ajoutez
# simplement les fichiers .c dans cette variable pour qu’ils soient compilés
# automatiquement.
SRCS    = main.c avl.c utils.c
OBJS    = $(SRCS:.c=.o)
TARGET  = c-wildwater

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Règles de compilation des fichiers objets
%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
