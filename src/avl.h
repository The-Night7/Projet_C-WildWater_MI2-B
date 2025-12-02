#ifndef AVL_H
#define AVL_H

#include <stdio.h>
#include "structs.h"

Station* insert_station(Station* node, char* name, long cap, long cons, long real);
void free_tree(Station* node);
void write_csv(Station* node, FILE* output, char* mode);

#endif