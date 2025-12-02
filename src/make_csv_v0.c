#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE1_PATH "data/c-wildwater_v0.dat"
#define OUTPUT_PATH "data/c-wildwater.csv"

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
        fprintf(output_file, "%s,%s,%s,%s,%.3f\n", factory_t, amont, aval, vol, pertes);
    } else {
        fprintf(stderr, "Erreur : Ligne mal formatée - %s\n", clean_line);
    }
}

int main()
{
    // Ouverture des fichiers en mode lecture et écriture
    FILE *in1 = fopen(FILE1_PATH, "r");
    if (!in1)
    {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s\n", FILE1_PATH);
        return 1;
    }

    FILE *out = fopen(OUTPUT_PATH, "w");
    if (!out)
    {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s\n", OUTPUT_PATH);
        fclose(in1);
        fclose(in2);
        return 1;
    }

    char line[256];

    // Écrire l'en-tête dans le fichier CSV
    fprintf(out, "Usine,Amont,Aval,Volume,Taux de perte\n");

    // Lecture du premier fichier et écriture dans le fichier CSV
    while (fgets(line, sizeof(line), in1))
    {
        if (strlen(line) > 1) { // Ignorer les lignes vides
        parse_and_write(line, out);
    }
    }

    // Lecture du second fichier et écriture dans le fichier CSV
    while (fgets(line, sizeof(line), in2))
    {
        if (strlen(line) > 1) { // Ignorer les lignes vides
        parse_and_write(line, out);
    }
    }

    // Fermeture des fichiers
    fclose(in1);
    fclose(out);

    printf("Traitement terminé avec succès.\n");
    return 0;
}