/**
 * @file utils.c
 * @brief Implémentation des fonctions utilitaires pour le projet C‑WildWater
 *
 * Cet ensemble de fonctions génériques n’est pas directement utilisé dans la
 * solution principale mais peut servir de base pour d’autres traitements
 * (par exemple lire un fichier CSV générique, stocker des données dans
 * un arbre AVL générique, etc.).
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int file_exists(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

// ----------------------------------------
// Fonctions AVL génériques

static int avl_get_height(AVLNode* node) {
    return node ? node->height : 0;
}

static int avl_get_balance(AVLNode* node) {
    return node ? avl_get_height(node->left) - avl_get_height(node->right) : 0;
}

static AVLNode* avl_right_rotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = MAX(avl_get_height(y->left), avl_get_height(y->right)) + 1;
    x->height = MAX(avl_get_height(x->left), avl_get_height(x->right)) + 1;
    return x;
}

static AVLNode* avl_left_rotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = MAX(avl_get_height(x->left), avl_get_height(x->right)) + 1;
    y->height = MAX(avl_get_height(y->left), avl_get_height(y->right)) + 1;
    return y;
}

AVLNode* avl_create_node(const char* key, void* value) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    if (!node) return NULL;
    node->key = strdup(key);
    node->value = value;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    return node;
}

AVLNode* avl_insert(AVLNode* root, const char* key, void* value) {
    if (!root) return avl_create_node(key, value);
    int cmp = strcmp(key, root->key);
    if (cmp < 0) {
        root->left = avl_insert(root->left, key, value);
    } else if (cmp > 0) {
        root->right = avl_insert(root->right, key, value);
    } else {
        // La clé existe déjà: on remplace la valeur
        root->value = value;
        return root;
    }
    // Mise à jour de la hauteur
    root->height = 1 + MAX(avl_get_height(root->left), avl_get_height(root->right));
    int balance = avl_get_balance(root);
    // Cas gauche gauche
    if (balance > 1 && strcmp(key, root->left->key) < 0) {
        return avl_right_rotate(root);
    }
    // Cas droite droite
    if (balance < -1 && strcmp(key, root->right->key) > 0) {
        return avl_left_rotate(root);
    }
    // Cas gauche droite
    if (balance > 1 && strcmp(key, root->left->key) > 0) {
        root->left = avl_left_rotate(root->left);
        return avl_right_rotate(root);
    }
    // Cas droite gauche
    if (balance < -1 && strcmp(key, root->right->key) < 0) {
        root->right = avl_right_rotate(root->right);
        return avl_left_rotate(root);
    }
    return root;
}

void* avl_search(AVLNode* root, const char* key) {
    if (!root) return NULL;
    int cmp = strcmp(key, root->key);
    if (cmp == 0) return root->value;
    if (cmp < 0) return avl_search(root->left, key);
    return avl_search(root->right, key);
}

void avl_destroy(AVLNode* root, void (*free_value)(void*)) {
    if (!root) return;
    avl_destroy(root->left, free_value);
    avl_destroy(root->right, free_value);
    free(root->key);
    if (free_value && root->value) {
        free_value(root->value);
    }
    free(root);
}

// ----------------------------------------
// Lecture de fichier CSV simple (séparateur virgule)

int read_data_file(const char* filepath,
                   void (*callback)(char**, int, void*),
                   void* user_data) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Erreur: impossible d’ouvrir le fichier %s\n", filepath);
        return 1;
    }
    char line[1024];
    char* token;
    char* tokens[16];
    int token_count;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        token_count = 0;
        token = strtok(line, ",");
        while (token && token_count < 16) {
            tokens[token_count++] = token;
            token = strtok(NULL, ",");
        }
        if (token_count > 0 && callback) {
            callback(tokens, token_count, user_data);
        }
    }
    fclose(file);
    return 0;
}
