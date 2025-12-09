/*
 * avl.c
 *
 * Implémentation de l’arbre AVL pour le projet C‑WildWater.  Cet
 * arbre est utilisé pour stocker les stations triées par identifiant et
 * agréger leurs différentes valeurs (capacité, volume capté et volume
 * réel).  De plus, chaque station possède une liste de connexions
 * sortantes afin de représenter le graphe d’écoulement utilisé lors
 * du calcul des fuites.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

// -----------------------------------------------------------------------------
//  Fonctions utilitaires internes
//

// Duplique une chaîne de caractères.  Retourne NULL en cas d’échec.
static char* my_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* new = malloc(len);
    if (new) memcpy(new, s, len);
    return new;
}

// Retourne la valeur maximale de deux entiers
static int max_int(int a, int b) {
    return (a > b) ? a : b;
}

// Retourne la hauteur d’un nœud ou 0 si celui‑ci est NULL
static int get_height(Station* n) {
    return n ? n->height : 0;
}

// Calcule le facteur d’équilibrage d’un nœud AVL
static int get_balance(Station* n) {
    return n ? get_height(n->left) - get_height(n->right) : 0;
}

// Crée un nouveau nœud de station
static Station* create_node(char* name) {
    Station* node = (Station*)malloc(sizeof(Station));
    if (!node) {
        fprintf(stderr, "Erreur: impossible d’allouer une nouvelle station\n");
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

// Effectue une rotation droite autour du nœud y
static Station* right_rotate(Station* y) {
    Station* x = y->left;
    Station* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max_int(get_height(y->left), get_height(y->right)) + 1;
    x->height = max_int(get_height(x->left), get_height(x->right)) + 1;
    return x;
}

// Effectue une rotation gauche autour du nœud x
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
//  Fonctions publiques
//

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
    if (!new_adj) {
        fprintf(stderr, "Erreur: impossible d’allouer une nouvelle connexion\n");
        exit(EXIT_FAILURE);
    }
    new_adj->target = child;
    new_adj->leak_perc = leak;
    new_adj->next = parent->children;
    parent->children = new_adj;
    parent->nb_children++;
}

// Insère ou met à jour une station dans l’arbre AVL.  Si le nœud existe
// déjà, ses champs capacity, consumption et real_qty sont incrémentés des
// valeurs fournies.  La comparaison est faite sur le nom de l’usine.
Station* insert_station(Station* node, char* name, long cap, long cons, long real) {
    if (!node) {
        Station* n = create_node(name);
        n->capacity = cap;
        n->consumption = cons;
        n->real_qty = real;
        return n;
    }
    int cmp = strcmp(name, node->name);
    if (cmp < 0) {
        node->left = insert_station(node->left, name, cap, cons, real);
    } else if (cmp > 0) {
        node->right = insert_station(node->right, name, cap, cons, real);
    } else {
        // Mise à jour des valeurs existantes
        node->capacity += cap;
        node->consumption += cons;
        node->real_qty += real;
        return node;
    }
    // Mise à jour de la hauteur et équilibrage
    node->height = 1 + max_int(get_height(node->left), get_height(node->right));
    int balance = get_balance(node);
    // Cas gauche gauche
    if (balance > 1 && strcmp(name, node->left->name) < 0) {
        return right_rotate(node);
    }
    // Cas droite droite
    if (balance < -1 && strcmp(name, node->right->name) > 0) {
        return left_rotate(node);
    }
    // Cas gauche droite
    if (balance > 1 && strcmp(name, node->left->name) > 0) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }
    // Cas droite gauche
    if (balance < -1 && strcmp(name, node->right->name) < 0) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }
    return node;
}

void free_tree(Station* node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    // Libérer la liste des connexions
    AdjNode* curr = node->children;
    while (curr) {
        AdjNode* tmp = curr;
        curr = curr->next;
        free(tmp);
    }
    free(node->name);
    free(node);
}

// Parcourt l’AVL et écrit les données demandées dans un fichier CSV.  Le
// paramètre `mode` doit être "max", "src" ou "real".  Pour chaque
// station ayant une valeur strictement positive, on écrit une ligne
// « identifiant;valeur » sur la sortie spécifiée.
void write_csv(Station* node, FILE* output, char* mode) {
    if (!node) return;
    write_csv(node->left, output, mode);
    double val = 0;
    if (strcmp(mode, "max") == 0) {
        val = node->capacity/1000.0;
    } else if (strcmp(mode, "src") == 0) {
        val = node->consumption/1000.0;
    } else if (strcmp(mode, "real") == 0) {
        val = node->real_qty/1000.0;
    }
    if (val > 0) {
        fprintf(output, "%s;%.6f\n", node->name, val);
    }
    write_csv(node->right, output, mode);
}
