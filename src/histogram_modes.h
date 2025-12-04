/**
 * @file histogram_modes.h
 * @brief Définition des modes d'histogramme pour le projet C-WildWater
 * @author Votre équipe
 * @date Décembre 2025
 */

#ifndef PROJET_C_WILDWATER_MI2_B_HISTOGRAM_MODES_H
#define PROJET_C_WILDWATER_MI2_B_HISTOGRAM_MODES_H

#include <string.h>

// Définition des modes d'histogramme
typedef enum {
    HISTO_MODE_UNKNOWN = 0, // Mode non reconnu
    HISTO_MODE_MAX = 1,     // Capacité maximale de traitement
    HISTO_MODE_SRC = 2,     // Volume total capté
    HISTO_MODE_REAL = 3,    // Volume total réellement traité
    HISTO_MODE_ALL = 4      // Tous les modes combinés (bonus)
} HistogramMode;

// Structure pour stocker les résultats d'histogramme
typedef struct {
    char* factory_id;    // Identifiant de l'usine
    double max_value;    // Capacité maximale de traitement
    double src_value;    // Volume total capté
    double real_value;   // Volume total réellement traité
} HistogramEntry;

/**
 * @brief Convertit une chaîne de caractères en mode d'histogramme
 * 
 * @param mode_str Chaîne représentant le mode ("max", "src", "real", "all")
 * @return HistogramMode Le mode correspondant, HISTO_MODE_UNKNOWN si non reconnu
 */
static inline HistogramMode string_to_histogram_mode(const char* mode_str) {
    if (!mode_str) return HISTO_MODE_UNKNOWN;
    if (strcmp(mode_str, "max") == 0) return HISTO_MODE_MAX;
    if (strcmp(mode_str, "src") == 0) return HISTO_MODE_SRC;
    if (strcmp(mode_str, "real") == 0) return HISTO_MODE_REAL;
    if (strcmp(mode_str, "all") == 0) return HISTO_MODE_ALL;
    return HISTO_MODE_UNKNOWN; // Mode non reconnu
}

/**
 * @brief Obtient le nom du mode d'histogramme
 * 
 * @param mode Mode d'histogramme
 * @return const char* Nom du mode
 */
static inline const char* histogram_mode_to_string(HistogramMode mode) {
    switch (mode) {
        case HISTO_MODE_MAX: return "max";
        case HISTO_MODE_SRC: return "src";
        case HISTO_MODE_REAL: return "real";
        case HISTO_MODE_ALL: return "all";
        default: return "unknown";
    }
}

/**
 * @brief Obtient la description du mode d'histogramme
 * 
 * @param mode Mode d'histogramme
 * @return const char* Description du mode
 */
static inline const char* get_histogram_mode_description(HistogramMode mode) {
    switch (mode) {
        case HISTO_MODE_MAX:
            return "Capacité maximale de traitement";
        case HISTO_MODE_SRC:
            return "Volume total capté";
        case HISTO_MODE_REAL:
            return "Volume total réellement traité";
        case HISTO_MODE_ALL:
            return "Tous les modes combinés";
        default:
            return "Mode inconnu";
    }
}

/**
 * @brief Obtient le nom du fichier de sortie pour un mode donné
 *
 * @param mode Mode d'histogramme
 * @return const char* Nom du fichier de sortie (sans extension)
 */
static inline const char* get_output_filename_base(HistogramMode mode) {
    switch (mode) {
        case HISTO_MODE_MAX: return "output_histo_max";
        case HISTO_MODE_SRC: return "output_histo_src";
        case HISTO_MODE_REAL: return "output_histo_real";
        case HISTO_MODE_ALL: return "output_histo_all";
        default: return "output_unknown";
    }
}

/**
 * @brief Vérifie si le mode d'histogramme est valide
 *
 * @param mode Mode d'histogramme à vérifier
 * @return int 1 si valide, 0 sinon
 */
static inline int is_valid_histogram_mode(HistogramMode mode) {
    return (mode >= HISTO_MODE_MAX && mode <= HISTO_MODE_ALL);
}

#endif // PROJET_C_WILDWATER_MI2_B_HISTOGRAM_MODES_H