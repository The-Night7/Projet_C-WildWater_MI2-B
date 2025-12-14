/*
* avl.h
 *
 * Interface d'un arbre AVL pour le projet C-WildWater.
 * Stocke des stations hydrauliques ordonnées par identifiant.
 */

#ifndef AVL_H
#define AVL_H

#include <stdio.h>
#include "structs.h"

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