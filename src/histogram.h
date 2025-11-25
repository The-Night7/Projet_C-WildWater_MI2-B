/**
 * @file histogram.h
 * @brief Module pour la génération des histogrammes
 * @author Votre équipe
 * @date Novembre 2025
 */

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

// Types d'histogrammes
#define HISTO_TYPE_MAX 1  // Capacité maximale
#define HISTO_TYPE_SRC 2  // Volume capté
#define HISTO_TYPE_REAL 3 // Volume réellement traité
#define HISTO_TYPE_ALL 4  // Les trois combinés (bonus)

/**
 * @brief Génère un histogramme basé sur les données d'eau
 * 
 * @param data_file_v0 Chemin vers le fichier de données v0
 * @param data_file_v3 Chemin vers le fichier de données v3
 * @param histo_type Type d'histogramme à générer (voir constantes HISTO_TYPE_*)
 * @return int Code de retour (0 en cas de succès, positif en cas d'erreur)
 */
int generate_histogram(const char* data_file_v0, const char* data_file_v3, int histo_type);

/**
 * @brief Sauvegarde les résultats de l'histogramme dans un fichier
 * 
 * @param results Tableau de résultats
 * @param count Nombre d'éléments dans le tableau
 * @param output_file Nom du fichier de sortie
 * @return int Code de retour (0 en cas de succès, positif en cas d'erreur)
 */
int save_histogram_results(void* results, int count, const char* output_file);

/**
 * @brief Génère une image PNG de l'histogramme en utilisant GnuPlot
 * 
 * @param data_file Fichier de données pour l'histogramme
 * @param output_image Nom du fichier image de sortie
 * @param title Titre de l'histogramme
 * @param histo_type Type d'histogramme
 * @return int Code de retour (0 en cas de succès, positif en cas d'erreur)
 */
int generate_histogram_image(const char* data_file, const char* output_image, const char* title, int histo_type);

#endif // HISTOGRAM_H