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
    int height;
    struct Station* left;
    struct Station* right;

    // Structure Graphe (pour les fuites)
    AdjNode* children;  // Liste des stations en aval
    int nb_children;    // Nombre d'enfants (pour diviser le flux équitablement)
} Station;

#endif