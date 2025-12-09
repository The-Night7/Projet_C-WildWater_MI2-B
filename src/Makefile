# ==========================================
# Makefile pour le projet C-WildWater
# ==========================================

# --- 1. Définition des Variables ---
# CC : Le compilateur à utiliser (gcc est standard pour le C)
CC = gcc

# CFLAGS (Compiler Flags) : Options de compilation
# -Wall -Wextra : Active presque tous les avertissements (très important pour un code propre)
# -std=c99      : Force l'utilisation du standard C99 (autorise les boucles for(int i...))
# -g            : Ajoute les symboles de débogage (indispensable pour utiliser gdb ou valgrind)
CFLAGS = -Wall -Wextra -std=c99 -g

# LDFLAGS (Linker Flags) : Options pour l'édition des liens
# -lm : Lie la bibliothèque mathématique standard (nécessaire si on utilise math.h)
LDFLAGS = -lm

# --- 2. Cible par défaut ---
# C'est la première règle lue par 'make'. Elle appelle la cible 'c-wildwater'.
all: c-wildwater

# --- 3. Édition des liens (Linker) ---
# Crée l'exécutable final à partir des fichiers objets (.o)
# $@ : Variable automatique qui représente le nom de la cible (ici "c-wildwater")
# $^ : Variable automatique qui représente toutes les dépendances (ici "main.o avl.o")
c-wildwater: main.o avl.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# --- 4. Compilation des fichiers sources ---

# Compilation de main.c en main.o
# Dépend de main.c mais aussi des headers (.h) : si on change une structure, on doit recompiler.
# -c : Indique à gcc de compiler sans faire l'édition de liens (génère un .o)
main.o: main.c avl.h structs.h
	$(CC) $(CFLAGS) -c main.c

# Compilation de avl.c en avl.o
# Dépend de avl.c et des fichiers d'en-tête associés.
avl.o: avl.c avl.h structs.h
	$(CC) $(CFLAGS) -c avl.c

# --- 5. Nettoyage ---
# Supprime les fichiers générés (.o et l'exécutable) pour repartir sur une base saine.
clean:
	rm -f *.o c-wildwater

# --- 6. Cibles spéciales ---
# .PHONY indique à make que 'all' et 'clean' ne sont pas des vrais fichiers.
# Cela évite les conflits si un fichier nommé "clean" existait par hasard dans le dossier.
.PHONY: all clean