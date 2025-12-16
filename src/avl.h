/*
* avl.h
 *
 * AVL tree implementation for storing hydraulic stations by identifier.
 */

#ifndef AVL_H
#define AVL_H

#include <stdio.h>
#include "structs.h"

/**
 * Inserts or updates a station in the AVL tree
 *
 * @param node  Root of the tree
 * @param name  Station identifier
 * @param cap   Capacity to add
 * @param cons  Consumption to add
 * @param real  Actual volume to add
 * @return      New tree root after insertion/balancing
 */
Station* insert_station(Station* node, char* name, long cap, long cons, long real);

/**
 * Searches for a station by its identifier
 *
 * @param node  Root of the tree
 * @param name  Identifier to search for
 * @return      Pointer to the station or NULL if not found
 */
Station* find_station(Station* node, char* name);

/**
 * Adds a connection between two stations
 *
 * @param parent  Source station
 * @param child   Destination station
 * @param leak    Leak percentage on this section
 * @param factory Factory associated with this connection
 */
void add_connection(Station* parent, Station* child, double leak, Station* factory);

/**
 * Frees the memory used by the tree and its connections
 *
 * @param node  Root of the tree to free
 */
void free_tree(Station* node);

/**
 * Generates a CSV file from the tree data
 *
 * @param node    Root of the tree
 * @param output  Output file
 * @param mode    Data type ("max", "src" or "real")
 */
void write_csv(Station* node, FILE* output, char* mode);

#endif /* AVL_H */