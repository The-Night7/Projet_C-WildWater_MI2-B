#ifndef AVL_H
#define AVL_H

#include <stdio.h>
#include "structs.h"

// Crée ou met à jour une station (Mode Histo)
Station* insert_station(Station* node, char* name, long cap, long cons, long real);

// Trouve une station existante (Mode Leaks)
Station* find_station(Station* node, char* name);

// Ajoute une connexion parent -> enfant (Mode Leaks)
void add_connection(Station* parent, Station* child, double leak);

// Libère tout
void free_tree(Station* node);

// Ecrit le CSV pour l'histogramme
void write_csv(Station* node, FILE* output, char* mode);

#endif