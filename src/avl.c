#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

// --- Utilitaires ---
char* my_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* new = malloc(len);
    if (new) memcpy(new, s, len);
    return new;
}

int max(int a, int b) { return (a > b) ? a : b; }

int get_height(Station* n) {
    if (!n) return 0;
    return n->height;
}

int get_balance(Station* n) {
    if (!n) return 0;
    return get_height(n->left) - get_height(n->right);
}

Station* create_node(char* name) {
    Station* node = (Station*)malloc(sizeof(Station));
    if (!node) exit(1);
    
    node->name = my_strdup(name);
    node->capacity = 0;
    node->consumption = 0;
    node->real_qty = 0;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    
    // Init Graphe
    node->children = NULL;
    node->nb_children = 0;
    
    return node;
}

// --- Rotations ---
Station* right_rotate(Station* y) {
    Station* x = y->left;
    Station* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(get_height(y->left), get_height(y->right)) + 1;
    x->height = max(get_height(x->left), get_height(x->right)) + 1;
    return x;
}

Station* left_rotate(Station* x) {
    Station* y = x->right;
    Station* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(get_height(x->left), get_height(x->right)) + 1;
    y->height = max(get_height(y->left), get_height(y->right)) + 1;
    return y;
}

// --- Fonctions Principales ---

Station* find_station(Station* node, char* name) {
    if (!node) return NULL;
    int cmp = strcmp(name, node->name);
    if (cmp == 0) return node;
    if (cmp < 0) return find_station(node->left, name);
    return find_station(node->right, name);
}

void add_connection(Station* parent, Station* child, double leak) {
    if (!parent || !child) return;
    
    AdjNode* new_adj = (AdjNode*)malloc(sizeof(AdjNode));
    if (!new_adj) exit(1);
    
    new_adj->target = child;
    new_adj->leak_perc = leak;
    new_adj->next = parent->children;
    parent->children = new_adj;
    parent->nb_children++;
}

Station* insert_station(Station* node, char* name, long cap, long cons, long real) {
    if (!node) {
        Station* n = create_node(name);
        n->capacity = cap;
        n->consumption = cons;
        n->real_qty = real;
        return n;
    }

    int cmp = strcmp(name, node->name);
    if (cmp < 0) node->left = insert_station(node->left, name, cap, cons, real);
    else if (cmp > 0) node->right = insert_station(node->right, name, cap, cons, real);
    else {
        // Mise à jour (somme des valeurs)
        node->capacity += cap;
        node->consumption += cons;
        node->real_qty += real;
        return node;
    }

    node->height = 1 + max(get_height(node->left), get_height(node->right));
    int balance = get_balance(node);

    if (balance > 1 && strcmp(name, node->left->name) < 0) return right_rotate(node);
    if (balance < -1 && strcmp(name, node->right->name) > 0) return left_rotate(node);
    if (balance > 1 && strcmp(name, node->left->name) > 0) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }
    if (balance < -1 && strcmp(name, node->right->name) < 0) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }
    return node;
}

void free_tree(Station* node) {
    if (node) {
        free_tree(node->left);
        free_tree(node->right);
        
        // Libération de la liste chaînée des enfants
        AdjNode* curr = node->children;
        while (curr) {
            AdjNode* tmp = curr;
            curr = curr->next;
            free(tmp);
        }
        
        free(node->name);
        free(node);
    }
}

void write_csv(Station* node, FILE* output, char* mode) {
    if (node) {
        write_csv(node->left, output, mode);
        
        long val = 0;
        if (strcmp(mode, "max") == 0) val = node->capacity;
        else if (strcmp(mode, "src") == 0) val = node->consumption;
        else if (strcmp(mode, "real") == 0) val = node->real_qty;

        if (val > 0) {
            fprintf(output, "%s;%ld\n", node->name, val);
        }
        write_csv(node->right, output, mode);
    }
}