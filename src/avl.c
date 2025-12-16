/*
 * avl.c
 *
 * Implementation of an AVL tree for storing hydraulic stations.
 * Manages station data and their interconnections.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

// -----------------------------------------------------------------------------
// Internal utility functions
// -----------------------------------------------------------------------------

/**
 * Creates a duplicate of a string
 *
 * @param s  String to duplicate
 * @return   New allocated string or NULL if allocation fails
 */
static char* my_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* new = malloc(len);
    if (new) memcpy(new, s, len);
    return new;
}

/**
 * Returns the maximum of two integers
 */
static int max_int(int a, int b) {
    return (a > b) ? a : b;
}

/**
 * Gets the height of a node (0 if NULL)
 */
static int get_height(Station* n) {
    return n ? n->height : 0;
}

/**
 * Calculates the balance factor of a node
 * Positive value means left-heavy, negative means right-heavy
 */
static int get_balance(Station* n) {
    return n ? get_height(n->left) - get_height(n->right) : 0;
}

/**
 * Creates a new initialized station node
 *
 * @param name  Station identifier
 * @return      Newly allocated station
 */
static Station* create_node(char* name) {
    Station* node = (Station*)malloc(sizeof(Station));
    if (!node) {
        fprintf(stderr, "Error: unable to allocate a new station\n");
        exit(EXIT_FAILURE);
    }
    node->name = my_strdup(name);
    node->capacity = 0;
    node->consumption = 0;
    node->real_qty = 0;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    node->children = NULL;
    node->nb_children = 0;
    return node;
}

/**
 * Performs a right rotation to rebalance the AVL tree
 *
 * @param y  Root node before rotation
 * @return   New root node after rotation
 */
static Station* right_rotate(Station* y) {
    Station* x = y->left;
    Station* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max_int(get_height(y->left), get_height(y->right)) + 1;
    x->height = max_int(get_height(x->left), get_height(x->right)) + 1;
    return x;
}

/**
 * Performs a left rotation to rebalance the AVL tree
 *
 * @param x  Root node before rotation
 * @return   New root node after rotation
 */
static Station* left_rotate(Station* x) {
    Station* y = x->right;
    Station* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max_int(get_height(x->left), get_height(x->right)) + 1;
    y->height = max_int(get_height(y->left), get_height(y->right)) + 1;
    return y;
}

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

/**
 * Searches for a station by name in the AVL tree
 *
 * @param node  Root of the tree
 * @param name  Station identifier to search for
 * @return      Pointer to the station or NULL if not found
 */
Station* find_station(Station* node, char* name) {
    if (!node) return NULL;
    int cmp = strcmp(name, node->name);
    if (cmp == 0) return node;
    if (cmp < 0) return find_station(node->left, name);
    return find_station(node->right, name);
}

/**
 * Adds a connection between two stations
 *
 * @param parent   Source station
 * @param child    Destination station
 * @param leak     Leak percentage on this section
 * @param factory  Factory associated with this connection
 */
void add_connection(Station* parent, Station* child, double leak, Station* factory) {
    if (!parent || !child) return;

    // Check if connection already exists for this factory
    AdjNode* check = parent->children;
    while (check) {
        if (check->target == child && check->factory == factory) {
            return; // Connection already exists
        }
        check = check->next;
    }

    // Create and initialize a new connection
    AdjNode* new_adj = (AdjNode*)malloc(sizeof(AdjNode));
    if (!new_adj) {
        fprintf(stderr, "Error: unable to allocate a new connection\n");
        exit(EXIT_FAILURE);
    }
    new_adj->target = child;
    new_adj->leak_perc = leak;
    new_adj->factory = factory;
    new_adj->next = parent->children;
    parent->children = new_adj;
    parent->nb_children++;
}

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
Station* insert_station(Station* node, char* name, long cap, long cons, long real) {
    // Base case: create a new node
    if (!node) {
        Station* n = create_node(name);
        n->capacity = cap;
        n->consumption = cons;
        n->real_qty = real;
        return n;
    }

    // Recursive search for insertion position
    int cmp = strcmp(name, node->name);
    if (cmp < 0) {
        node->left = insert_station(node->left, name, cap, cons, real);
    } else if (cmp > 0) {
        node->right = insert_station(node->right, name, cap, cons, real);
    } else {
        // Existing station: update values
        node->capacity += cap;
        node->consumption += cons;
        node->real_qty += real;
        return node;
    }

    // Update height
    node->height = 1 + max_int(get_height(node->left), get_height(node->right));

    // Check balance and rotate if necessary
    int balance = get_balance(node);

    // Four possible imbalance cases
    if (balance > 1 && strcmp(name, node->left->name) < 0) {
        return right_rotate(node);  // Left-Left case
    }
    if (balance < -1 && strcmp(name, node->right->name) > 0) {
        return left_rotate(node);   // Right-Right case
    }
    if (balance > 1 && strcmp(name, node->left->name) > 0) {
        node->left = left_rotate(node->left);  // Left-Right case
        return right_rotate(node);
    }
    if (balance < -1 && strcmp(name, node->right->name) < 0) {
        node->right = right_rotate(node->right);  // Right-Left case
        return left_rotate(node);
    }

    return node;
}

/**
 * Frees memory used by the tree and its connections
 *
 * @param node  Root of the tree to free
 */
void free_tree(Station* node) {
    if (!node) return;

    // Recursive freeing of subtrees
    free_tree(node->left);
    free_tree(node->right);

    // Free the connection list
    AdjNode* curr = node->children;
    while (curr) {
        AdjNode* tmp = curr;
        curr = curr->next;
        free(tmp);
    }

    free(node->name);
    free(node);
}

/**
 * Writes station data to a CSV file according to the specified mode
 *
 * @param node    Root of the tree
 * @param output  Output file
 * @param mode    Data type ("max", "src", "real", or "all")
 */
void write_csv(Station* node, FILE* output, char* mode) {
    if (!node) return;

    // Inorder traversal (left-root-right)
    write_csv(node->left, output, mode);

    if (strcmp(mode, "all") == 0) {
        // Mode "all": display all three values on the same line
        double max_val = node->capacity/1000.0;
        double src_val = node->consumption/1000.0;
        double real_val = node->real_qty/1000.0;

        // Write only if at least one value is positive
        if (max_val > 0 || src_val > 0 || real_val > 0) {
            fprintf(output, "%s;%.6f;%.6f;%.6f\n",
                   node->name, max_val, src_val, real_val);
        }
    } else {
        // Standard modes (max, src, real)
        double val = 0;
        if (strcmp(mode, "max") == 0) {
            val = node->capacity/1000.0;        // Maximum capacity
        } else if (strcmp(mode, "src") == 0) {
            val = node->consumption/1000.0;     // Captured volume
        } else if (strcmp(mode, "real") == 0) {
            val = node->real_qty/1000.0;        // Actual volume
        }

        // Write only positive values
        if (val > 0) {
            fprintf(output, "%s;%.6f\n", node->name, val);
        }
    }

    write_csv(node->right, output, mode);
}