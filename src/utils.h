/**
 * @file utils.h
 * @brief Fonctions utilitaires pour le projet C‑WildWater
 *
 * Ce fichier fournit quelques fonctions génériques telles qu’une
 * implémentation minimaliste d’un arbre AVL générique et une fonction
 * permettant de vérifier l’existence d’un fichier.  Ces utilitaires ne
 * sont pas utilisés directement dans la solution principale mais sont
 * laissés ici pour une éventuelle extension.
 */

#ifndef UTILS_H
#define UTILS_H

// Structure de nœud pour un AVL générique (clé/valeur)
typedef struct AVLNode {
    char* key;            // Identifiant de la station
    void* value;          // Pointeur générique vers des données
    int height;           // Hauteur du nœud
    struct AVLNode* left; // Sous‑arbre gauche
    struct AVLNode* right;// Sous‑arbre droit
} AVLNode;

// Vérifie si un fichier existe (renvoie 1 si présent, 0 sinon)
int file_exists(const char* filepath);

// Créé un nouveau nœud AVL générique
AVLNode* avl_create_node(const char* key, void* value);

// Insère ou remplace un nœud dans l’AVL générique
AVLNode* avl_insert(AVLNode* root, const char* key, void* value);

// Recherche une clé dans l’AVL générique et retourne la valeur
void* avl_search(AVLNode* root, const char* key);

// Libère l’AVL générique.  La fonction free_value est appelée sur chaque
// valeur si elle est fournie.
void avl_destroy(AVLNode* root, void (*free_value)(void*));

// Lit un fichier CSV simple et appelle un callback pour chaque ligne
int read_data_file(const char* filepath,
                   void (*callback)(char**, int, void*),
                   void* user_data);

#endif /* UTILS_H */
