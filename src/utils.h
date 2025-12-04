/**
 * @file utils.h
 * @brief Fonctions utilitaires pour le projet C-WildWater
 * @author Votre équipe
 * @date Novembre 2025
 */

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h> // Pour size_t

// Structure de données AVL pour stocker les identifiants d'usines
typedef struct AVLNode {
    char* key;
    void* value; // Pointeur générique vers FactoryData
    struct AVLNode *left;
    struct AVLNode *right;
    int height;
} AVLNode;

/**
 * @brief Vérifie si un fichier existe
 * 
 * @param filepath Chemin du fichier à vérifier
 * @return int 1 si le fichier existe, 0 sinon
 */
int file_exists(const char* filepath);

/**
 * @brief Crée un nouveau nœud AVL
 * 
 * @param key Clé du nœud
 * @param value Valeur associée
 * @return AVLNode* Pointeur vers le nœud créé, NULL en cas d'échec
 */
AVLNode* avl_create_node(const char* key, void* value);

/**
 * @brief Insère un élément dans un arbre AVL
 * 
 * @param root Racine de l'arbre AVL
 * @param key Clé à insérer
 * @param value Valeur associée
 * @return AVLNode* Nouvelle racine de l'arbre après insertion
 */
AVLNode* avl_insert(AVLNode* root, const char* key, void* value);

/**
 * @brief Recherche un élément dans un arbre AVL
 * 
 * @param root Racine de l'arbre AVL
 * @param key Clé à rechercher
 * @return void* Valeur associée à la clé, NULL si non trouvée
 */
AVLNode* avl_search(AVLNode* root, const char* key);

/**
 * @brief Libère la mémoire utilisée par un arbre AVL
 * 
 * @param root Racine de l'arbre AVL
 * @param free_value Fonction pour libérer les valeurs (NULL si non nécessaire)
 */
void avl_destroy(AVLNode* root, void (*free_value)(void*));

/**
 * @brief Lit les données des fichiers CSV
 * 
 * @param filepath Chemin du fichier à lire
 * @param callback Fonction de rappel appelée pour chaque ligne
 * @param user_data Données utilisateur passées à la fonction de rappel
 * @return int 0 en cas de succès, code d'erreur sinon
 */
int read_data_file(const char* filepath, void (*callback)(char**, int, void*), void* user_data);

/**
 * @brief Supprime les espaces et retours à la ligne au début et à la fin d'une chaîne.
 *
 * @param str La chaîne à nettoyer.
 */
void trim_whitespace(char *str);

/**
 * @brief Vérifie si une chaîne est vide ou nulle.
 *
 * @param str La chaîne à vérifier.
 * @return 1 si vide, 0 sinon.
 */
int is_empty(const char *str);

#endif // UTILS_H