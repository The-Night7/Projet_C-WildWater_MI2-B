/*
 * avl.h
 *
 * Interface de l’arbre AVL spécialisé pour le projet C‑WildWater.
 * Chaque nœud de l’arbre stocke une structure `Station`.  L’arbre est
 * ordonné par identifiant (chaîne de caractères).  Les fonctions
 * d’insertion agrègent automatiquement les valeurs passées en paramètre
 * lorsque l’identifiant existe déjà.
 */

#ifndef AVL_H
#define AVL_H

#include <stdio.h>
#include "structs.h"

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
