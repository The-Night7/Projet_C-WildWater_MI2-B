/*
* Data structures for the C-WildWater project
 *
 * Defines structures to represent the hydraulic network
 * as both an AVL tree and a directed graph.
 */

#ifndef STRUCTS_H
#define STRUCTS_H

/**
 * Linked list node for connections between stations
 */
typedef struct AdjNode {
    struct Station* target;   // Destination station
    double leak_perc;         // Leak percentage on this section
    struct Station* factory;  // Facility associated with this section
    struct AdjNode* next;     // Pointer to next node
} AdjNode;

/**
 * Hydraulic station (facility, source, storage, etc.)
 * Serves as both a node in the AVL tree and a vertex in the graph
 */
typedef struct Station {
    char* name;           // Unique identifier

    // Volume data (in internal units)
    long capacity;        // Maximum processing capacity
    long consumption;     // Volume captured upstream
    long real_qty;        // Actual volume after losses

    // AVL tree fields
    int height;           // Height of subtree
    struct Station* left; // Left subtree
    struct Station* right;// Right subtree

    // Flow graph fields
    AdjNode* children;    // List of outgoing connections
    int nb_children;      // Number of outgoing connections
} Station;

/**
 * Structure for parallel leak calculation tasks
 */
typedef struct {
    Station* node;            // Station to process
    double input_vol;         // Input volume
    Station* facility;        // Target facility
    double* leak_result;      // Pointer to store result
    double* max_leak_val;     // Pointer to track maximum leak
    char** max_from;          // Pointer to track upstream station of critical section
    char** max_to;            // Pointer to track downstream station of critical section
} LeakTaskData;

#endif /* STRUCTS_H */