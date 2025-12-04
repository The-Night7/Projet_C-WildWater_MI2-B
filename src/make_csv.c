#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "make_csv.h"

// Fonction pour analyser une ligne et écrire dans le fichier CSV
void parse_and_write(const char *input_line, FILE *output_file)
{
    char factory_t[50], amont[50], aval[50], vol[50];
    float pertes;

    // Supprimer le saut de ligne à la fin de la ligne
    char clean_line[256];
    strncpy(clean_line, input_line, sizeof(clean_line));
    clean_line[strcspn(clean_line, "\n")] = '\0';

    // Utiliser sscanf pour extraire les champs selon le format des lignes
    int parsed = sscanf(clean_line, "%49[^;];%49[^;];%49[^;];%49[^;];%f",
                        factory_t, amont, aval, vol, &pertes);

    // Vérifier si tous les champs ont été correctement analysés
    if (parsed == 5) {
        fprintf(output_file, "%s;%s;%s;%s;%.3f\n", factory_t, amont, aval, vol, pertes);
    } else {
        fprintf(stderr, "Erreur : Ligne mal formatée - %s\n", clean_line);
    }
}

// Fonction pour convertir un fichier .dat en .csv
int convert_dat_to_csv_file(const char* input_file, const char* output_file)
{
    // Ouverture des fichiers en mode lecture et écriture
    FILE *in = fopen(input_file, "r");
    if (!in)
    {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s\n", input_file);
        return 1;
    }

    FILE *out = fopen(output_file, "w");
    if (!out)
    {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s\n", output_file);
        fclose(in);
        return 1;
    }

    char line[256];

    // Écrire l'en-tête dans le fichier CSV
    fprintf(out, "Station;Amont;Aval;Volume;Fuite\n");

    // Lecture du fichier et écriture dans le fichier CSV
    while (fgets(line, sizeof(line), in))
    {
        if (strlen(line) > 1) { // Ignorer les lignes vides
        parse_and_write(line, out);
    }
    }

    // Fermeture des fichiers
    fclose(in);
    fclose(out);

    return 0;
}