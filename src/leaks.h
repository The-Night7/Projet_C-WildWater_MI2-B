/**
 * @file leaks.h
 * @brief Module pour le calcul des pertes d'eau
 * @author Votre équipe
 * @date Novembre 2025
 */

#ifndef LEAKS_H
#define LEAKS_H

/**
 * @brief Calcule les pertes d'eau pour une usine spécifique
 * 
 * @param data_file_v0 Chemin vers le fichier de données v0
 * @param data_file_v3 Chemin vers le fichier de données v3
 * @param plant_id Identifiant de l'usine
 * @return int Code de retour (0 en cas de succès, positif en cas d'erreur)
 */
int calculate_leaks(const char* data_file_v0, const char* data_file_v3, const char* plant_id);

/**
 * @brief Ajoute un enregistrement à l'historique des pertes
 * 
 * @param plant_id Identifiant de l'usine
 * @param leak_volume Volume de pertes en millions de m³
 * @return int Code de retour (0 en cas de succès, positif en cas d'erreur)
 */
int add_to_leaks_history(const char* plant_id, double leak_volume);

/**
 * @brief Trouve le tronçon avec la plus grande perte d'eau (bonus)
 * 
 * @param data_file_v0 Chemin vers le fichier de données v0
 * @param data_file_v3 Chemin vers le fichier de données v3
 * @return int Code de retour (0 en cas de succès, positif en cas d'erreur)
 */
int find_max_leak_segment(const char* data_file_v0, const char* data_file_v3);

#endif // LEAKS_H