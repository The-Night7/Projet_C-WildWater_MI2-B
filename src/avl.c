#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

// --- Utilitaires ---

// Crée une copie d'une chaîne de caractères en allouant dynamiquement la mémoire.
// C'est nécessaire car "line" dans le main est écrasé à chaque lecture.
// Il faut donc sauvegarder le nom de la station dans un espace mémoire dédié.
char* my_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;       // +1 pour le caractère de fin de chaîne '\0'
    char* new = malloc(len);          // Allocation
    if (new) memcpy(new, s, len);     // Copie
    return new;
}

// Fonction utilitaire simple pour obtenir le maximum entre deux entiers
// Utilisée pour calculer la hauteur de l'arbre AVL.
int max(int a, int b) { return (a > b) ? a : b; }

// Retourne la hauteur d'un nœud de manière sécurisée (gère le cas NULL)
int get_height(Station* n) {
    if (!n) return 0; // Une feuille ou un nœud vide a une hauteur de 0
    return n->height;
}

// Calcule le facteur d'équilibre (Balance Factor)
// Si le résultat est > 1 ou < -1, l'arbre est déséquilibré et nécessite une rotation.
int get_balance(Station* n) {
    if (!n) return 0;
    return get_height(n->left) - get_height(n->right);
}

// Initialise une nouvelle Station avec toutes ses valeurs à 0.
// Cette fonction est appelée la première fois qu'on rencontre un nouvel identifiant.
Station* create_node(char* name) {
    Station* node = (Station*)malloc(sizeof(Station));
    if (!node) exit(1); // Arrêt d'urgence si plus de mémoire (très rare)
    
    node->name = my_strdup(name); // Copie profonde du nom
    node->capacity = 0;
    node->consumption = 0;
    node->real_qty = 0;
    node->height = 1;             // Un nouveau nœud commence avec hauteur 1
    node->left = NULL;
    node->right = NULL;
    
    // Init Graphe (Liste chaînée pour les fuites)
    node->children = NULL;        // Pointeur de tête de liste
    node->nb_children = 0;
    
    return node;
}
// --- Rotations ---

// Rotation Droite (Right Rotate)
// Utilisée quand le sous-arbre GAUCHE est trop lourd (déséquilibre > 1).
// 'y' est la racine actuelle, 'x' va devenir la nouvelle racine.
Station* right_rotate(Station* y) {
    Station* x = y->left;
    Station* T2 = x->right; // T2 est le sous-arbre qui va changer de parent

    // La rotation
    x->right = y;
    y->left = T2;

    // Mise à jour des hauteurs (il faut commencer par y car il est maintenant en dessous de x)
    y->height = max(get_height(y->left), get_height(y->right)) + 1;
    x->height = max(get_height(x->left), get_height(x->right)) + 1;

    return x; // x est la nouvelle racine de ce sous-arbre
}

// Rotation Gauche (Left Rotate)
// Utilisée quand le sous-arbre DROIT est trop lourd (déséquilibre < -1).
// Miroir exact de la rotation droite.
Station* left_rotate(Station* x) {
    Station* y = x->right;
    Station* T2 = y->left;

    // La rotation
    y->left = x;
    x->right = T2;

    // Mise à jour des hauteurs
    x->height = max(get_height(x->left), get_height(x->right)) + 1;
    y->height = max(get_height(y->left), get_height(y->right)) + 1;

    return y; // y est la nouvelle racine
}
// --- Fonctions Principales ---

// Recherche binaire récursive (Complexité O(log n) grâce à l'AVL).
// Renvoie le pointeur vers la station si trouvée, sinon NULL.
Station* find_station(Station* node, char* name) {
    if (!node) return NULL; // Pas trouvé
    
    int cmp = strcmp(name, node->name);
    
    if (cmp == 0) return node; // Trouvé !
    if (cmp < 0) return find_station(node->left, name);  // Chercher à gauche (plus petit)
    return find_station(node->right, name);              // Chercher à droite (plus grand)
}

// Ajoute une connexion dans le graphe pour le calcul des fuites.
// Ajoute un nœud en tête de la liste chaînée 'children' du parent.
void add_connection(Station* parent, Station* child, double leak) {
    if (!parent || !child) return;
    
    // Allocation d'un maillon de liste chaînée (AdjNode)
    AdjNode* new_adj = (AdjNode*)malloc(sizeof(AdjNode));
    if (!new_adj) exit(1);
    
    new_adj->target = child;        // Pointeur vers la station enfant (déjà dans l'AVL)
    new_adj->leak_perc = leak;      // Stockage du % de fuite
    
    // Insertion en tête de liste
    new_adj->next = parent->children;
    parent->children = new_adj;
    
    parent->nb_children++;          // Incrémenter pour la division du flux plus tard
}

// Insère une station ou met à jour ses données si elle existe déjà.
// Gère l'équilibrage AVL automatique après insertion.
Station* insert_station(Station* node, char* name, long cap, long cons, long real) {
    // 1. Insertion normale (BST)
    if (!node) {
        // Si la station n'existe pas, on la crée avec les valeurs fournies
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
        // CAS IMPORTANT : La station existe déjà (doublon d'ID).
        // On ne crée pas de nouveau nœud, on cumule les volumes (somme).
        node->capacity += cap;
        node->consumption += cons;
        node->real_qty += real;
        return node;
    }

    // 2. Mise à jour de la hauteur de l'ancêtre actuel
    node->height = 1 + max(get_height(node->left), get_height(node->right));

    // 3. Vérification de l'équilibre (Balance Factor)
    int balance = get_balance(node);

    // 4. Les 4 cas de rééquilibrage si nécessaire

    // Cas Gauche-Gauche (LL)
    if (balance > 1 && strcmp(name, node->left->name) < 0) return right_rotate(node);

    // Cas Droite-Droite (RR)
    if (balance < -1 && strcmp(name, node->right->name) > 0) return left_rotate(node);

    // Cas Gauche-Droite (LR)
    if (balance > 1 && strcmp(name, node->left->name) > 0) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    // Cas Droite-Gauche (RL)
    if (balance < -1 && strcmp(name, node->right->name) < 0) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node; // Retourne le nœud (potentiellement nouvelle racine après rotation)
}
// Libère TOUTE la mémoire récursivement (Arbre + Listes chaînées + Noms).
// Indispensable pour éviter les fuites mémoire (valgrind).
void free_tree(Station* node) {
    if (node) {
        // Appel récursif sur les sous-arbres
        free_tree(node->left);
        free_tree(node->right);
        
        // Libération de la liste chaînée des enfants (spécifique au mode Leaks)
        AdjNode* curr = node->children;
        while (curr) {
            AdjNode* tmp = curr;
            curr = curr->next;
            free(tmp); // On libère le maillon AdjNode, mais PAS la station cible (target)
        }
        
        free(node->name); // Libérer la chaîne de caractères (malloc dans create_node)
        free(node);       // Libérer la structure Station elle-même
    }
}

// Parcours Infixe (In-Order Traversal) : Gauche -> Racine -> Droite
// Cela permet de sortir les stations triées par ordre alphabétique croissant.
void write_csv(Station* node, FILE* output, char* mode) {
    if (node) {
        write_csv(node->left, output, mode); // D'abord les plus petits
        
        long val = 0;
        // Sélection de la valeur à afficher selon l'argument du shell
        if (strcmp(mode, "max") == 0) val = node->capacity;
        else if (strcmp(mode, "src") == 0) val = node->consumption;
        else if (strcmp(mode, "real") == 0) val = node->real_qty;

        // On n'écrit que si la valeur est pertinente (positive)
        // Cela filtre les stations qui ne sont pas concernées par le mode choisi
        if (val > 0) {
            fprintf(output, "%s;%ld\n", node->name, val);
        }
        
        write_csv(node->right, output, mode); // Ensuite les plus grands
    }
}