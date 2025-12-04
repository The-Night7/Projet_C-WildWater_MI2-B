#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv_io.h"

// Déclaration de la fonction de conversion
char* convert_dat_to_csv(const char* dat_file);

int main(int argc, char *argv[]) {
    // Vérification des arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file.csv ou input_file.dat>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_file = argv[1];
    char *csv_file = NULL;
    int need_free = 0;

    printf("Starting C-WildWater analysis...\n");

    // Vérifier l'extension du fichier
    const char *ext = strrchr(input_file, '.');
    if (ext && strcmp(ext, ".dat") == 0) {
        printf("Fichier .dat détecté, conversion en CSV...\n");
        csv_file = convert_dat_to_csv(input_file);
        if (!csv_file) {
            fprintf(stderr, "Erreur lors de la conversion du fichier .dat en .csv\n");
            return EXIT_FAILURE;
        }
        need_free = 1;
    } else {
        // Utiliser directement le fichier d'entrée s'il est déjà au format CSV
        csv_file = (char*)input_file;
    }

    // Appel de la fonction de traitement du CSV
    process_csv_file(csv_file);

    // Libérer la mémoire si nécessaire
    if (need_free && csv_file) {
        free(csv_file);
    }

    printf("Analysis complete.\n");
    return EXIT_SUCCESS;
}

// Fonction pour convertir un fichier .dat en .csv
char* convert_dat_to_csv(const char* dat_file) {
    // Créer un nom de fichier temporaire pour le CSV
    char* csv_file = malloc(strlen(dat_file) + 5); // +5 pour ".csv\0"
    if (!csv_file) return NULL;

    strcpy(csv_file, dat_file);
    char* ext = strrchr(csv_file, '.');
    if (ext) *ext = '\0'; // Supprimer l'extension existante
    strcat(csv_file, ".csv");

    // Ouvrir le fichier .dat en lecture
    FILE* in = fopen(dat_file, "r");
    if (!in) {
        free(csv_file);
        return NULL;
    }

    // Ouvrir le fichier .csv en écriture
    FILE* out = fopen(csv_file, "w");
    if (!out) {
        fclose(in);
        free(csv_file);
        return NULL;
    }

    // Écrire l'en-tête dans le fichier CSV
    fprintf(out, "Station;Amont;Aval;Volume;Fuite\n");

    // Lire et convertir chaque ligne
    char line[1024];
    while (fgets(line, sizeof(line), in)) {
        // Supprimer le saut de ligne
        line[strcspn(line, "\n")] = '\0';

        // Ignorer les lignes vides
        if (strlen(line) <= 1) continue;

        // Supposons que le format .dat utilise des points-virgules comme séparateurs
        // Si ce n'est pas le cas, il faudrait adapter cette partie
        fprintf(out, "%s\n", line);
    }

    fclose(in);
    fclose(out);
    return csv_file;
}
