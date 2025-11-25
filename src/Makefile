# Makefile pour le projet C-WildWater

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lm

# Chemins des dossiers
PROJECT_ROOT = ..
DATA_DIR = $(PROJECT_ROOT)/data
TESTS_DIR = $(PROJECT_ROOT)/tests
# Fichiers source et objets
SRCS = main.c histogram.c leaks.c utils.c
OBJS = $(SRCS:.c=.o)
HEADERS = histogram.h leaks.h utils.h

# Nom de l'exécutable
TARGET = c-wildwater
# Règle par défaut
all: $(TARGET)
# Compilation de l'exécutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
	@echo "Compilation terminée : $(TARGET)"

# Règle générique pour compiler les fichiers objets
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers générés
clean:
	rm -f $(OBJS) *~ *.bak *.tmp
	@echo "Fichiers objets et temporaires supprimés"

# Nettoyage complet
distclean: clean
	rm -f $(TARGET)
	@echo "Exécutable supprimé"

# Exécution avec données par défaut
run: $(TARGET)
	./$(TARGET) $(DATA_DIR)/c-wildwater_v0.dat $(DATA_DIR)/c-wildwater_v3.dat

# Règle pour les tests
test: $(TARGET)
	cp $(TARGET) $(TESTS_DIR)/
	cd $(TESTS_DIR) && bash ./run_tests.sh
	@echo "Tests terminés"

# Installation (copie l'exécutable dans le répertoire bin s'il existe)
install: $(TARGET)
	mkdir -p $(PROJECT_ROOT)/bin
	cp $(TARGET) $(PROJECT_ROOT)/bin/
	@echo "Installation terminée"

# Règles de dépendances explicites
main.o: main.c histogram.h leaks.h utils.h
histogram.o: histogram.c histogram.h utils.h
leaks.o: leaks.c leaks.h utils.h
utils.o: utils.c utils.h

# Afficher l'aide
help:
	@echo "Makefile pour le projet C-WildWater"
	@echo "Commandes disponibles :"
	@echo "  make          - Compile le programme"
	@echo "  make clean    - Supprime les fichiers objets"
	@echo "  make distclean - Supprime les fichiers objets et l'exécutable"
	@echo "  make run      - Exécute le programme avec les données par défaut"
	@echo "  make test     - Exécute les tests automatisés"
	@echo "  make install  - Installe l'exécutable dans le dossier bin"
	@echo "  make help     - Affiche cette aide"
.PHONY: all clean distclean run test install help