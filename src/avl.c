/*
 * avl.c
 *
 * Implémentation d'un arbre AVL pour le projet C-WildWater.
 * Stocke les stations hydrauliques avec leurs données et leurs connexions,
 * permettant de modéliser le graphe d'écoulement pour le calcul des fuites.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

// -----------------------------------------------------------------------------
// Fonctions utilitaires internes
// -----------------------------------------------------------------------------

// Duplique une chaîne de caractères
static char* my_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* new = malloc(len);
    if (new) memcpy(new, s, len);
    return new;
}

// Retourne le maximum de deux entiers
static int max_int(int a, int b) {
    return (a > b) ? a : b;
}

// Retourne la hauteur d'un nœud (0 si NULL)
static int get_height(Station* n) {
    return n ? n->height : 0;
}

// Calcule le facteur d'équilibrage d'un nœud
static int get_balance(Station* n) {
    return n ? get_height(n->left) - get_height(n->right) : 0;
}

// Crée un nouveau nœud de station initialisé
static Station* create_node(char* name) {
    Station* node = (Station*)malloc(sizeof(Station));
    if (!node) {
        fprintf(stderr, "Erreur: impossible d'allouer une nouvelle station\n");
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

// Rotation droite pour rééquilibrer l'arbre AVL
static Station* right_rotate(Station* y) {
    Station* x = y->left;
    Station* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max_int(get_height(y->left), get_height(y->right)) + 1;
    x->height = max_int(get_height(x->left), get_height(x->right)) + 1;
    return x;
}

// Rotation gauche pour rééquilibrer l'arbre AVL
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
// Fonctions publiques
// -----------------------------------------------------------------------------

// Recherche une station par son nom dans l'arbre AVL
Station* find_station(Station* node, char* name) {
    if (!node) return NULL;
    int cmp = strcmp(name, node->name);
    if (cmp == 0) return node;
    if (cmp < 0) return find_station(node->left, name);
    return find_station(node->right, name);
}

// Ajoute une connexion entre deux stations pour une usine spécifique
void add_connection(Station* parent, Station* child, double leak, Station* factory) {
    if (!parent || !child) return;

    // Vérifier si la connexion existe déjà pour cette usine
    AdjNode* check = parent->children;
    while (check) {
        if (check->target == child && check->factory == factory) {
            return; // Connexion déjà existante
        }
        check = check->next;
    }

    // Créer et initialiser une nouvelle connexion
    AdjNode* new_adj = (AdjNode*)malloc(sizeof(AdjNode));
    if (!new_adj) {
        fprintf(stderr, "Erreur: impossible d'allouer une nouvelle connexion\n");
        exit(EXIT_FAILURE);
    }
    new_adj->target = child;
    new_adj->leak_perc = leak;
    new_adj->factory = factory;
    new_adj->next = parent->children;
    parent->children = new_adj;
    parent->nb_children++;
}

// Insère ou met à jour une station dans l'arbre AVL
Station* insert_station(Station* node, char* name, long cap, long cons, long real) {
    // Cas de base: création d'un nouveau nœud
    if (!node) {
        Station* n = create_node(name);
        n->capacity = cap;
        n->consumption = cons;
        n->real_qty = real;
        return n;
    }

    // Recherche récursive de la position d'insertion
    int cmp = strcmp(name, node->name);
    if (cmp < 0) {
        node->left = insert_station(node->left, name, cap, cons, real);
    } else if (cmp > 0) {
        node->right = insert_station(node->right, name, cap, cons, real);
    } else {
        // Station existante: mise à jour des valeurs
        node->capacity += cap;
        node->consumption += cons;
        node->real_qty += real;
        return node;
    }

    // Mise à jour de la hauteur
    node->height = 1 + max_int(get_height(node->left), get_height(node->right));

    // Vérification de l'équilibre et rotations si nécessaire
    int balance = get_balance(node);

    // Quatre cas de déséquilibre possibles
    if (balance > 1 && strcmp(name, node->left->name) < 0) {
        return right_rotate(node);  // Cas gauche-gauche
    }
    if (balance < -1 && strcmp(name, node->right->name) > 0) {
        return left_rotate(node);   // Cas droite-droite
    }
    if (balance > 1 && strcmp(name, node->left->name) > 0) {
        node->left = left_rotate(node->left);  // Cas gauche-droite
        return right_rotate(node);
    }
    if (balance < -1 && strcmp(name, node->right->name) < 0) {
        node->right = right_rotate(node->right);  // Cas droite-gauche
        return left_rotate(node);
    }

    return node;
}

// Libère la mémoire utilisée par l'arbre et ses connexions
void free_tree(Station* node) {
    if (!node) return;

    // Libération récursive des sous-arbres
    free_tree(node->left);
    free_tree(node->right);

    // Libération de la liste des connexions
    AdjNode* curr = node->children;
    while (curr) {
        AdjNode* tmp = curr;
        curr = curr->next;
        free(tmp);
    }

    free(node->name);
    free(node);
}

// Écrit les données des stations dans un fichier CSV selon le mode spécifié
void write_csv(Station* node, FILE* output, char* mode) {
    if (!node) return;

    // Parcours infixe (gauche-racine-droite)
    write_csv(node->left, output, mode);

    // Sélection de la valeur selon le mode
    double val = 0;
    if (strcmp(mode, "max") == 0) {
        val = node->capacity/1000.0;        // Capacité maximale
    } else if (strcmp(mode, "src") == 0) {
        val = node->consumption/1000.0;     // Volume capté
    } else if (strcmp(mode, "real") == 0) {
        val = node->real_qty/1000.0;        // Volume réel
    }

    // Écriture uniquement des valeurs positives
    if (val > 0) {
        fprintf(output, "%s;%.6f\n", node->name, val);
    }

    write_csv(node->right, output, mode);
}