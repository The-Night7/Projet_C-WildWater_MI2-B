/**
 * @file main.c
 * @brief Point d'entrée du programme C-WildWater pour l'analyse de données sur l'eau
 * @author Votre équipe
 * @date Novembre 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "histogram.h"
#include "leaks.h"
#include "utils.h"

#define EXIT_SUCCESS 0
#define EXIT_INVALID_ARGS 1
#define EXIT_FILE_ERROR 2
#define EXIT_MEMORY_ERROR 3

void print_usage(const char* program_name) {
    fprintf(stderr, "Usage: %s <fichier_donnees_v0> <fichier_donnees_v3> <mode> [options]\n", program_name);
    fprintf(stderr, "Modes disponibles :\n");
    fprintf(stderr, "  histo max       : Histogramme basé sur la capacité maximale\n");
    fprintf(stderr, "  histo src       : Histogramme basé sur le volume capté\n");
    fprintf(stderr, "  histo real      : Histogramme basé sur le volume réellement traité\n");
    fprintf(stderr, "  histo all       : Histogramme cumulé des trois valeurs (bonus)\n");
    fprintf(stderr, "  leaks <id_usine>: Calcul des pertes pour une usine spécifique\n");
}

int main(int argc, char* argv[]) {
    // Vérifier le nombre minimum d'arguments (nom du programme + 2 fichiers + mode)
    if (argc < 4) {
        fprintf(stderr, "Erreur: Arguments insuffisants\n");
        print_usage(argv[0]);
        return EXIT_INVALID_ARGS;
    }

    // Récupérer les chemins des fichiers de données
    char* data_file_v0 = argv[1];
    char* data_file_v3 = argv[2];
    char* mode = argv[3];

    // Vérifier l'existence des fichiers de données
    if (!file_exists(data_file_v0) || !file_exists(data_file_v3)) {
        fprintf(stderr, "Erreur: Un ou plusieurs fichiers de données introuvables\n");
        return EXIT_FILE_ERROR;
    }

    // Traitement selon le mode
    if (strcmp(mode, "histo") == 0) {
        if (argc < 5) {
            fprintf(stderr, "Erreur: Type d'histogramme non spécifié\n");
            print_usage(argv[0]);
            return EXIT_INVALID_ARGS;
        }

        char* histo_type = argv[4];
        
        // Traitement selon le type d'histogramme
        if (strcmp(histo_type, "max") == 0) {
            return generate_histogram(data_file_v0, data_file_v3, HISTO_TYPE_MAX);
        } else if (strcmp(histo_type, "src") == 0) {
            return generate_histogram(data_file_v0, data_file_v3, HISTO_TYPE_SRC);
        } else if (strcmp(histo_type, "real") == 0) {
            return generate_histogram(data_file_v0, data_file_v3, HISTO_TYPE_REAL);
        } else if (strcmp(histo_type, "all") == 0) {
            return generate_histogram(data_file_v0, data_file_v3, HISTO_TYPE_ALL);
        } else {
            fprintf(stderr, "Erreur: Type d'histogramme invalide: %s\n", histo_type);
            print_usage(argv[0]);
            return EXIT_INVALID_ARGS;
        }
    } else if (strcmp(mode, "leaks") == 0) {
        if (argc < 5) {
            fprintf(stderr, "Erreur: Identifiant d'usine non spécifié\n");
            print_usage(argv[0]);
            return EXIT_INVALID_ARGS;
        }

        char* plant_id = argv[4];
        return calculate_leaks(data_file_v0, data_file_v3, plant_id);
    } else {
        fprintf(stderr, "Erreur: Mode invalide: %s\n", mode);
        print_usage(argv[0]);
        return EXIT_INVALID_ARGS;
    }

    return EXIT_SUCCESS;
}