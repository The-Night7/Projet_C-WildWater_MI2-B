/*
 * Structures de données pour le projet C-WildWater
 *
 * Définit les structures pour représenter le réseau hydraulique
 * sous forme d'arbre AVL et de graphe orienté.
 */

#ifndef STRUCTS_H
#define STRUCTS_H

/**
 * Nœud de liste chaînée pour les connexions entre stations
 */
typedef struct AdjNode {
    struct Station* target;   // Station destination
    double leak_perc;         // Pourcentage de fuite sur ce tronçon
    struct Station* factory;  // Usine associée à ce tronçon
    struct AdjNode* next;     // Pointeur vers le nœud suivant
} AdjNode;

/**
 * Station hydraulique (usine, source, stockage, etc.)
 * Sert à la fois de nœud dans l'arbre AVL et de sommet dans le graphe
 */
typedef struct Station {
    char* name;           // Identifiant unique

    // Données volumétriques (en unités internes)
    long capacity;        // Capacité maximale de traitement
    long consumption;     // Volume capté en amont
    long real_qty;        // Volume réel après pertes

    // Champs pour l'arbre AVL
    int height;           // Hauteur du sous-arbre
    struct Station* left; // Sous-arbre gauche
    struct Station* right;// Sous-arbre droit

    // Champs pour le graphe d'écoulement
    AdjNode* children;    // Liste des connexions sortantes
    int nb_children;      // Nombre de connexions sortantes
} Station;

#endif /* STRUCTS_H */