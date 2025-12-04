#ifndef CSV_IO_H
#define CSV_IO_H

#include <stdio.h>
#include "utils.h" // Nécessaire pour utiliser le type AVLNode

/**
 * @brief Lit le fichier CSV d'entrée et remplit l'arbre AVL avec les données.
 * 
 * @param filepath Le chemin vers le fichier CSV source (ex: data/data.csv).
 * @param root La racine de l'arbre AVL (peut être NULL au début).
 * @param mode Le mode de fonctionnement (1=max, 2=src, 3=real, etc.).
 * @return La nouvelle racine de l'arbre AVL après insertion.
 */
AVLNode* process_input_csv(const char* filepath, AVLNode* root, int mode);

/**
 * @brief Parcourt l'arbre AVL et écrit les résultats dans le fichier CSV de sortie.
 * 
 * @param filepath Le chemin vers le fichier de sortie (ex: data/output.csv).
 * @param root La racine de l'arbre AVL contenant les données agrégées.
 * @param mode Le mode de fonctionnement pour savoir quelle donnée écrire.
 */
void generate_output_csv(const char* filepath, AVLNode* root, int mode);

/**
 * @brief Traite un fichier CSV ligne par ligne.
 *
 * @param filename Le chemin du fichier à ouvrir.
 */
void process_csv_file(const char *filename);

#endif // CSV_IO_H
