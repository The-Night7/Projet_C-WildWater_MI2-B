<<<<<<< HEAD
#ifndef STRUCTS_H
#define STRUCTS_H

// Noeud de liste chaînée pour les connexions (Enfants)
typedef struct AdjNode {
    struct Station* target; // Pointeur vers la station enfant
    double leak_perc;       // Pourcentage de fuite sur ce tuyau
    struct AdjNode* next;
} AdjNode;

typedef struct Station {
    // Données d'identification
    char* name;         

    // Données Histo
    long capacity;      // Capacité max (Usine)
    long consumption;   // Volume capté (Source)
    long real_qty;      // Volume réel après fuite (Source->Usine)
    
    // Structure AVL
=======
/*
 * Structures de données pour le projet C‑WildWater
 *
 * La structure `Station` représente une usine de traitement ou une entité du
 * réseau (source, stockage, etc.).  Elle est utilisée à la fois comme nœud
 * d’un arbre AVL (pour trier les usines par identifiant) et comme
 * sommet d’un graphe orienté (pour calculer les pertes).  Les structures
 * définies ici sont identiques à celles utilisées dans les fichiers
 * originaux fournis avec le sujet.
 */

#ifndef STRUCTS_H
#define STRUCTS_H

// Nœud de liste chaînée pour les connexions (enfants dans le graphe)
typedef struct AdjNode {
    struct Station* target;   // Pointeur vers la station enfant
    double leak_perc;         // Pourcentage de fuite sur ce tronçon
    struct AdjNode* next;     // Maillon suivant de la liste
} AdjNode;

// Nœud représentant une station/usine dans l’arbre AVL et dans le graphe
typedef struct Station {
    char* name;           // Identifiant unique de l’usine

    // Données agrégées pour l’histogramme
    long capacity;        // Capacité maximale (usine)
    long consumption;     // Volume capté (sources vers usine)
    long real_qty;        // Volume réel après fuite (sources vers usine)

    // Champs de l’AVL
>>>>>>> origin/teuteu_test
    int height;
    struct Station* left;
    struct Station* right;

<<<<<<< HEAD
    // Structure Graphe (pour les fuites)
    AdjNode* children;  // Liste des stations en aval
    int nb_children;    // Nombre d'enfants (pour diviser le flux équitablement)
} Station;

#endif
=======
    // Champs pour le graphe de fuites
    AdjNode* children;    // Liste des stations en aval
    int nb_children;      // Nombre d’enfants (utilisé pour répartir le flux)
} Station;

#endif /* STRUCTS_H */
>>>>>>> origin/teuteu_test
