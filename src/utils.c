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

int file_exists(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

// Fonctions AVL
int get_height(AVLNode* node) {
    if (node == NULL) return 0;
    return node->height;
}

int get_balance(AVLNode* node) {
    if (node == NULL) return 0;
    return get_height(node->left) - get_height(node->right);
}

AVLNode* right_rotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = MAX(get_height(y->left), get_height(y->right)) + 1;
    x->height = MAX(get_height(x->left), get_height(x->right)) + 1;

    return x;
}

AVLNode* left_rotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = MAX(get_height(x->left), get_height(x->right)) + 1;
    y->height = MAX(get_height(y->left), get_height(y->right)) + 1;

    return y;
}

AVLNode* avl_create_node(const char* key, void* value) {
    AVLNode* node = (AVLNode*) malloc(sizeof(AVLNode));
    if (node == NULL) return NULL;

    node->key = strdup(key);
    node->value = value;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    return node;
}

AVLNode* avl_insert(AVLNode* root, const char* key, void* value) {
    // Insertion standard BST
    if (root == NULL)
        return avl_create_node(key, value);

    int cmp = strcmp(key, root->key);
    if (cmp < 0)
        root->left = avl_insert(root->left, key, value);
    else if (cmp > 0)
        root->right = avl_insert(root->right, key, value);
    else {
        // Clé déjà présente, mettre à jour la valeur
        root->value = value;
        return root;
    }

    // Mise à jour de la hauteur
    root->height = 1 + MAX(get_height(root->left), get_height(root->right));

    // Vérification de l'équilibre
    int balance = get_balance(root);

    // Cas de déséquilibre
    // Cas gauche-gauche
    if (balance > 1 && strcmp(key, root->left->key) < 0)
        return right_rotate(root);

    // Cas droite-droite
    if (balance < -1 && strcmp(key, root->right->key) > 0)
        return left_rotate(root);

    // Cas gauche-droite
    if (balance > 1 && strcmp(key, root->left->key) > 0) {
        root->left = left_rotate(root->left);
        return right_rotate(root);
    }

    // Cas droite-gauche
    if (balance < -1 && strcmp(key, root->right->key) < 0) {
        root->right = right_rotate(root->right);
        return left_rotate(root);
    }

    return root;
}

AVLNode* avl_search(AVLNode* root, const char* key) {
    if (root == NULL || strcmp(key, root->key) == 0)
        return root;

    if (strcmp(key, root->key) < 0)
        return avl_search(root->left, key);

    return avl_search(root->right, key);
}


void avl_destroy(AVLNode* root, void (*free_value)(void*)) {
    if (root == NULL) return;

    // Détruire les sous-arbres
    avl_destroy(root->left, free_value);
    avl_destroy(root->right, free_value);

    // Libérer la mémoire du nœud
    free(root->key);
    if (free_value != NULL && root->value != NULL)
        free_value(root->value);
    free(root);
}

// Fonctions de lecture de fichier
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