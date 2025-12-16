<<<<<<< HEAD
<<<<<<< HEAD
=======
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

>>>>>>> origin/teuteu_test
=======
/*
 * avl.c
 *
 * Implémentation d'un arbre AVL pour le projet C-WildWater.
 * Stocke les stations hydrauliques avec leurs données et leurs connexions,
 * permettant de modéliser le graphe d'écoulement pour le calcul des fuites.
 */

>>>>>>> origin/main
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

<<<<<<< HEAD
<<<<<<< HEAD
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
=======
// -----------------------------------------------------------------------------
//  Fonctions utilitaires internes
//

// Duplique une chaîne de caractères.  Retourne NULL en cas d’échec.
=======
// -----------------------------------------------------------------------------
// Fonctions utilitaires internes
// -----------------------------------------------------------------------------

// Duplique une chaîne de caractères
>>>>>>> origin/main
static char* my_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* new = malloc(len);
    if (new) memcpy(new, s, len);
    return new;
}

<<<<<<< HEAD
// Retourne la valeur maximale de deux entiers
=======
// Retourne le maximum de deux entiers
>>>>>>> origin/main
static int max_int(int a, int b) {
    return (a > b) ? a : b;
}

<<<<<<< HEAD
// Retourne la hauteur d’un nœud ou 0 si celui‑ci est NULL
=======
// Retourne la hauteur d'un nœud (0 si NULL)
>>>>>>> origin/main
static int get_height(Station* n) {
    return n ? n->height : 0;
}

<<<<<<< HEAD
// Calcule le facteur d’équilibrage d’un nœud AVL
=======
// Calcule le facteur d'équilibrage d'un nœud
>>>>>>> origin/main
static int get_balance(Station* n) {
    return n ? get_height(n->left) - get_height(n->right) : 0;
}

<<<<<<< HEAD
// Crée un nouveau nœud de station
static Station* create_node(char* name) {
    Station* node = (Station*)malloc(sizeof(Station));
    if (!node) {
        fprintf(stderr, "Erreur: impossible d’allouer une nouvelle station\n");
=======
// Crée un nouveau nœud de station initialisé
static Station* create_node(char* name) {
    Station* node = (Station*)malloc(sizeof(Station));
    if (!node) {
        fprintf(stderr, "Erreur: impossible d'allouer une nouvelle station\n");
>>>>>>> origin/main
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

<<<<<<< HEAD
// Effectue une rotation droite autour du nœud y
=======
// Rotation droite pour rééquilibrer l'arbre AVL
>>>>>>> origin/main
static Station* right_rotate(Station* y) {
    Station* x = y->left;
    Station* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max_int(get_height(y->left), get_height(y->right)) + 1;
    x->height = max_int(get_height(x->left), get_height(x->right)) + 1;
    return x;
}

<<<<<<< HEAD
// Effectue une rotation gauche autour du nœud x
=======
// Rotation gauche pour rééquilibrer l'arbre AVL
>>>>>>> origin/main
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
<<<<<<< HEAD
//  Fonctions publiques
//

=======
// Fonctions publiques
// -----------------------------------------------------------------------------

// Recherche une station par son nom dans l'arbre AVL
>>>>>>> origin/main
Station* find_station(Station* node, char* name) {
    if (!node) return NULL;
    int cmp = strcmp(name, node->name);
    if (cmp == 0) return node;
    if (cmp < 0) return find_station(node->left, name);
    return find_station(node->right, name);
}

<<<<<<< HEAD
void add_connection(Station* parent, Station* child, double leak) {
    if (!parent || !child) return;
    AdjNode* check = parent->children;
    while (check) {
        // Si on trouve déjà ce voisin dans la liste, on arrête.
        // On compare les adresses mémoires (pointeurs) car l'AVL garantit l'unicité des noeuds.
        if (check->target == child) {
            return; // Déjà connecté, on ne fait rien
        }
        check = check->next;
    }
    AdjNode* new_adj = (AdjNode*)malloc(sizeof(AdjNode));
    if (!new_adj) {
        fprintf(stderr, "Erreur: impossible d’allouer une nouvelle connexion\n");
=======
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
>>>>>>> origin/main
        exit(EXIT_FAILURE);
    }
    new_adj->target = child;
    new_adj->leak_perc = leak;
<<<<<<< HEAD
=======
    new_adj->factory = factory;
>>>>>>> origin/main
    new_adj->next = parent->children;
    parent->children = new_adj;
    parent->nb_children++;
}

<<<<<<< HEAD
// Insère ou met à jour une station dans l’arbre AVL.  Si le nœud existe
// déjà, ses champs capacity, consumption et real_qty sont incrémentés des
// valeurs fournies.  La comparaison est faite sur le nom de l’usine.
Station* insert_station(Station* node, char* name, long cap, long cons, long real) {
    if (!node) {
>>>>>>> origin/teuteu_test
=======
// Insère ou met à jour une station dans l'arbre AVL
Station* insert_station(Station* node, char* name, long cap, long cons, long real) {
    // Cas de base: création d'un nouveau nœud
    if (!node) {
>>>>>>> origin/main
        Station* n = create_node(name);
        n->capacity = cap;
        n->consumption = cons;
        n->real_qty = real;
        return n;
    }
<<<<<<< HEAD
<<<<<<< HEAD

    int cmp = strcmp(name, node->name);
    if (cmp < 0) node->left = insert_station(node->left, name, cap, cons, real);
    else if (cmp > 0) node->right = insert_station(node->right, name, cap, cons, real);
    else {
        // CAS IMPORTANT : La station existe déjà (doublon d'ID).
        // On ne crée pas de nouveau nœud, on cumule les volumes (somme).
=======
=======

    // Recherche récursive de la position d'insertion
>>>>>>> origin/main
    int cmp = strcmp(name, node->name);
    if (cmp < 0) {
        node->left = insert_station(node->left, name, cap, cons, real);
    } else if (cmp > 0) {
        node->right = insert_station(node->right, name, cap, cons, real);
    } else {
<<<<<<< HEAD
        // Mise à jour des valeurs existantes
>>>>>>> origin/teuteu_test
=======
        // Station existante: mise à jour des valeurs
>>>>>>> origin/main
        node->capacity += cap;
        node->consumption += cons;
        node->real_qty += real;
        return node;
    }
<<<<<<< HEAD
<<<<<<< HEAD

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
=======
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
>>>>>>> origin/teuteu_test
    if (balance > 1 && strcmp(name, node->left->name) > 0) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }
<<<<<<< HEAD

    // Cas Droite-Gauche (RL)
=======
    // Cas droite gauche
>>>>>>> origin/teuteu_test
    if (balance < -1 && strcmp(name, node->right->name) < 0) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }
<<<<<<< HEAD

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
=======
    return node;
}

void free_tree(Station* node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    // Libérer la liste des connexions
=======

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
>>>>>>> origin/main
    AdjNode* curr = node->children;
    while (curr) {
        AdjNode* tmp = curr;
        curr = curr->next;
        free(tmp);
    }
<<<<<<< HEAD
=======

>>>>>>> origin/main
    free(node->name);
    free(node);
}

<<<<<<< HEAD
// Parcourt l’AVL et écrit les données demandées dans un fichier CSV.  Le
// paramètre `mode` doit être "max", "src" ou "real".  Pour chaque
// station ayant une valeur strictement positive, on écrit une ligne
// « identifiant;valeur » sur la sortie spécifiée.
// Parcourt l’AVL et écrit les données demandées dans un fichier CSV.
void write_csv(Station* node, FILE* output, char* mode) {
    if (!node) return;
    write_csv(node->left, output, mode);

    if (strcmp(mode, "all") == 0) {
        // Mode BONUS : on écrit les 3 valeurs (Capacité, Source, Réel)
        // On n'affiche que si l'usine a au moins une donnée pertinente
        if (node->capacity > 0 || node->consumption > 0 || node->real_qty > 0) {
            fprintf(output, "%s;%.6f;%.6f;%.6f\n", 
                    node->name, 
                    node->capacity/1000.0, 
                    node->consumption/1000.0, 
                    node->real_qty/1000.0);
        }
    } else {
        // Modes classiques (max, src, real)
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
    }
    write_csv(node->right, output, mode);
}
>>>>>>> origin/teuteu_test
=======
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
>>>>>>> origin/main
