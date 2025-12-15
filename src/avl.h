<<<<<<< HEAD
<<<<<<< HEAD
=======
/*
 * avl.h
 *
 * Interface de l’arbre AVL spécialisé pour le projet C‑WildWater.
 * Chaque nœud de l’arbre stocke une structure `Station`.  L’arbre est
 * ordonné par identifiant (chaîne de caractères).  Les fonctions
 * d’insertion agrègent automatiquement les valeurs passées en paramètre
 * lorsque l’identifiant existe déjà.
 */

>>>>>>> origin/teuteu_test
=======
/*
* avl.h
 *
 * Interface d'un arbre AVL pour le projet C-WildWater.
 * Stocke des stations hydrauliques ordonnées par identifiant.
 */

>>>>>>> origin/main
#ifndef AVL_H
#define AVL_H

#include <stdio.h>
#include "structs.h"

<<<<<<< HEAD
<<<<<<< HEAD
// Crée ou met à jour une station (Mode Histo)
Station* insert_station(Station* node, char* name, long cap, long cons, long real);

// Trouve une station existante (Mode Leaks)
Station* find_station(Station* node, char* name);

// Ajoute une connexion parent -> enfant (Mode Leaks)
void add_connection(Station* parent, Station* child, double leak);

// Libère tout
void free_tree(Station* node);

// Ecrit le CSV pour l'histogramme
void write_csv(Station* node, FILE* output, char* mode);

#endif
=======
// Insère ou met à jour une station dans l’arbre.  Si l’identifiant existe
// déjà, les valeurs de capacité, consommation et réel sont cumulées.
Station* insert_station(Station* node, char* name, long cap, long cons, long real);

// Recherche une station existante par son identifiant.  Renvoie NULL si
// l’identifiant n’est pas présent.
Station* find_station(Station* node, char* name);

// Ajoute une connexion parent→enfant dans le graphe de fuites.  La
// variable `leak` représente le pourcentage de fuites sur ce tronçon.
void add_connection(Station* parent, Station* child, double leak);

// Libère récursivement la mémoire occupée par l’arbre AVL et les listes
// chaînées des connexions.
void free_tree(Station* node);

// Parcourt l’AVL et écrit un fichier CSV pour l’histogramme selon le mode
// demandé.  Le paramètre `mode` doit être "max", "src" ou "real".  Pour
// chaque station ayant une valeur strictement positive, on écrit une ligne
// « identifiant;valeur » sur la sortie fournie.
void write_csv(Station* node, FILE* output, char* mode);

#endif /* AVL_H */
>>>>>>> origin/teuteu_test
=======
/**
 * Insère ou met à jour une station dans l'arbre AVL
 * @param node  Racine de l'arbre
 * @param name  Identifiant de la station
 * @param cap   Capacité à ajouter
 * @param cons  Consommation à ajouter
 * @param real  Volume réel à ajouter
 * @return      Nouvelle racine de l'arbre après insertion/équilibrage
 */
Station* insert_station(Station* node, char* name, long cap, long cons, long real);

/**
 * Recherche une station par son identifiant
 * @param node  Racine de l'arbre
 * @param name  Identifiant à rechercher
 * @return      Pointeur vers la station ou NULL si non trouvée
 */
Station* find_station(Station* node, char* name);

/**
 * Ajoute une connexion entre deux stations
 * @param parent  Station source
 * @param child   Station destination
 * @param leak    Pourcentage de fuite sur ce tronçon
 * @param factory Usine associée à cette connexion
 */
void add_connection(Station* parent, Station* child, double leak, Station* factory);

/**
 * Libère la mémoire de l'arbre et de ses connexions
 * @param node  Racine de l'arbre à libérer
 */
void free_tree(Station* node);

/**
 * Génère un fichier CSV à partir des données de l'arbre
 * @param node    Racine de l'arbre
 * @param output  Fichier de sortie
 * @param mode    Type de données ("max", "src" ou "real")
 */
void write_csv(Station* node, FILE* output, char* mode);

#endif /* AVL_H */
>>>>>>> origin/main
