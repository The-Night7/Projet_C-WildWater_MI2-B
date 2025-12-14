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
    /*
     * Usine à laquelle appartient ce tronçon.  Certaines lignes du
     * fichier CSV indiquent en première colonne l’identifiant de
     * l’usine qui a traité l’eau pour ce tronçon (stockage→jonction,
     * jonction→raccordement, raccordement→usager).  Pour les lignes
     * source→usine et usine→stockage, cette colonne est vide, mais
     * l’usine est implicite: respectivement l’usine aval et l’usine amont.
     * Ce champ est utilisé lors du calcul des fuites pour ne suivre
     * que les tronçons correspondant à l’usine étudiée.
     */
    struct Station* factory;
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
    int height;
    struct Station* left;
    struct Station* right;

    // Champs pour le graphe de fuites
    AdjNode* children;    // Liste des stations en aval
    int nb_children;      // Nombre d’enfants (utilisé pour répartir le flux)
} Station;

#endif /* STRUCTS_H */
