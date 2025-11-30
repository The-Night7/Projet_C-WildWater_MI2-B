# Makefile pour le projet C-WildWater

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lm

# Chemins des dossiers
PROJECT_ROOT = ..
SRC_DIR = $(PROJECT_ROOT)/src
DATA_DIR = $(PROJECT_ROOT)/data

# Fichiers source et objets (temporairement utils.c uniquement)
SRCS = $(SRC_DIR)/utils.c
OBJS = $(SRCS:.c=.o)

# Nom de l'exécutable
TARGET = c-wildwater

# Règle par défaut
all: $(TARGET)

# Compilation de l'exécutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
	@echo "Compilation terminée : $(TARGET)"

# Règle générique pour compiler les fichiers objets
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers générés
clean:
	rm -f $(SRC_DIR)/*.o *~ *.bak *.tmp
	@echo "Fichiers objets et temporaires supprimés"

# Nettoyage complet
distclean: clean
	rm -f $(TARGET)
	@echo "Exécutable supprimé"

# Exécution avec données par défaut
run: $(TARGET)
	./$(TARGET) $(DATA_DIR)/c-wildwater_v0.dat $(DATA_DIR)/c-wildwater_v3.dat

# Afficher l'aide
help:
	@echo "Makefile pour le projet C-WildWater"
	@echo "Commandes disponibles :"
	@echo "  make          - Compile le programme"
	@echo "  make clean    - Supprime les fichiers objets"
	@echo "  make distclean - Supprime les fichiers objets et l'exécutable"
	@echo "  make run      - Exécute le programme avec les données par défaut"
	@echo "  make help     - Affiche cette aide"

.PHONY: all clean distclean run help