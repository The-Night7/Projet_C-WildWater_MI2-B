/**
 * @file utils.c
 * @brief Implémentation des fonctions utilitaires pour le projet C-WildWater
 * @author Votre équipe
 * @date Novembre 2025
 */

#define _POSIX_C_SOURCE 200809L // nécessaire pour éviter un tas de problèmes potentiels à l'avenir

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

// --- Fonctions utilitaires AVL standards ---

int max(int a, int b) {
    return (a > b) ? a : b;
}
int get_height(AVLNode* node) {
    if (node == NULL) return 0;
    return node->height;
}

int get_balance(AVLNode* node) {
    if (node == NULL) return 0;
    return get_height(node->left) - get_height(node->right);
}

AVLNode* create_node(FactoryData* data) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    if (!node) {
        perror("Erreur allocation mémoire noeud");
        exit(EXIT_FAILURE);
    }
    node->data = data;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

// --- Rotations ---

AVLNode* right_rotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(get_height(y->left), get_height(y->right)) + 1;
    x->height = max(get_height(x->left), get_height(x->right)) + 1;

    return x;
}

AVLNode* left_rotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(get_height(x->left), get_height(x->right)) + 1;
    y->height = max(get_height(y->left), get_height(y->right)) + 1;

    return y;
}

// --- Logique de comparaison ---

// Retourne 1 si data1 > data2, 0 sinon (selon le mode)
int compare_factory_data(FactoryData* d1, FactoryData* d2, int mode) {
    double v1 = 0, v2 = 0;

    // Sélection de la valeur à comparer
    if (mode == HISTO_MODE_MAX) {
        v1 = d1->capacity;
        v2 = d2->capacity;
    } else if (mode == HISTO_MODE_SRC) {
        v1 = d1->load_volume;
        v2 = d2->load_volume;
    } else if (mode == HISTO_MODE_REAL) {
        v1 = d1->real_volume;
        v2 = d2->real_volume;
    } else {
        // Par défaut ou Mode ALL : on peut trier par capacité ou ID
        v1 = d1->capacity;
        v2 = d2->capacity;
    }

    // Comparaison numérique
    if (v1 > v2) return 1;
    if (v1 < v2) return -1;

    // Si égalité, on utilise l'ID pour différencier (éviter les doublons stricts)
    if (d1->id > d2->id) return 1;
    if (d1->id < d2->id) return -1;

    return 0;
}

// --- Insertion ---

AVLNode* insert_avl(AVLNode* node, FactoryData* data, int mode) {
    if (node == NULL) return create_node(data);

    int cmp = compare_factory_data(data, node->data, mode);
    if (cmp < 0)
        node->left = insert_avl(node->left, data, mode);
    else if (cmp > 0)
        node->right = insert_avl(node->right, data, mode);
    else
        return node; // Doublon exact ignoré
    // Mise à jour hauteur
    node->height = 1 + max(get_height(node->left), get_height(node->right));

    // Équilibrage
    int balance = get_balance(node);

    // Cas Left Left
    if (balance > 1 && compare_factory_data(data, node->left->data, mode) < 0)
        return right_rotate(node);

    // Cas Right Right
    if (balance < -1 && compare_factory_data(data, node->right->data, mode) > 0)
        return left_rotate(node);

    // Cas Left Right
    if (balance > 1 && compare_factory_data(data, node->left->data, mode) > 0) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    // Cas Right Left
    if (balance < -1 && compare_factory_data(data, node->right->data, mode) < 0) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

void avl_destroy(AVLNode* root, void (*free_data)(void*)) {
    if (root == NULL) return;
    avl_destroy(root->left, free_data);
    avl_destroy(root->right, free_data);
    if (free_data) free_data(root->data);
    free(root);
}// Fonctions de lecture de fichier

int file_exists(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}
int read_data_file(const char* filepath, void (*callback)(char**, int, void*), void* user_data) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier %s\n", filepath);
        return 1;
    }

    char line[1024];
    char* token;
    char* tokens[16]; // Supposons un maximum de 16 colonnes
    int token_count;

    // Lire les lignes une par une
    while (fgets(line, sizeof(line), file)) {
        // Supprimer le caractère de nouvelle ligne
        line[strcspn(line, "\n")] = '\0';

        // Tokeniser la ligne
        token_count = 0;
        token = strtok(line, ",");
        while (token && token_count < 16) {
            tokens[token_count++] = token;
            token = strtok(NULL, ",");
        }

        // Appeler la fonction de callback avec les tokens
        if (token_count > 0 && callback) {
            callback(tokens, token_count, user_data);
        }
    }

    fclose(file);
    return 0;
}

void trim_whitespace(char *str) {
    if (str == NULL) return;

    // Pointeur vers la fin de la chaîne
    char *end;

    // Supprimer les espaces au début : on avance le pointeur
    // Note : Cette implémentation modifie la chaîne en place mais ne déplace pas la mémoire,
    // pour une version plus simple, on nettoie souvent juste la fin.
    // Ici, on va se concentrer sur la fin (newline) qui pose souvent problème en CSV.

    // Suppression des espaces à la fin
    end = str + strlen(str) - 1;
    while (end > str && (isspace((unsigned char)*end) || *end == '\n' || *end == '\r')) {
        end--;
    }
    // Marquer la nouvelle fin de chaîne
    *(end + 1) = '\0';
}

int is_empty(const char *str) {
    if (str == NULL || str[0] == '\0') {
        return 1;
    }
    return 0;
}