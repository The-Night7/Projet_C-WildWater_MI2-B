# Makefile pour le projet C-WildWater

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -g

# Fichiers sources et objets
# On liste ici tous les fichiers .c nécessaires
SRCS = main.c utils.c csv_io.c make_csv.c
OBJS = $(SRCS:.c=.o)

# Nom de l'exécutable final
TARGET = c-wildwater

# Règle par défaut
all: $(TARGET)

# Édition de liens (Linking)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compilation des fichiers objets (.c -> .o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers temporaires
clean:
	rm -f $(OBJS) $(TARGET)

# Règle pour forcer la recompilation
re: clean all

.PHONY: all clean re
