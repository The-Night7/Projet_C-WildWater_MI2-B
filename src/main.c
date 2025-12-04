#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv_io.h"
#include "utils.h"
#include "histogram.h"

// Déclaration de la fonction de conversion
char* convert_dat_to_csv(const char* dat_file);

// Fonction pour libérer la mémoire des données de l'usine
void free_factory_data(void* data) {
    free(data);
}

int main(int argc, char *argv[]) {
    // Vérification des arguments
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <fichier_donnees.dat> <mode>\n", argv[0]);
        fprintf(stderr, "Modes disponibles:\n");
        fprintf(stderr, "  max  - Analyse des capacités maximales\n");
        fprintf(stderr, "  src  - Analyse des volumes traités\n");
        fprintf(stderr, "  real - Analyse des volumes réels consommés\n");
        return EXIT_FAILURE;
    }

    const char *input_file = argv[1];
    const char *mode_str = argv[2];
    int mode = 0;

    // Déterminer le mode d'analyse
    if (strcmp(mode_str, "max") == 0) {
        mode = 1;
    } else if (strcmp(mode_str, "src") == 0) {
        mode = 2;
    } else if (strcmp(mode_str, "real") == 0) {
        mode = 3;
    } else {
        fprintf(stderr, "Mode non reconnu: %s\n", mode_str);
        return EXIT_FAILURE;
    }

    char *csv_file = NULL;
    int need_free = 0;

    printf("Starting C-WildWater analysis (mode: %s)...\n", mode_str);

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

    // Initialisation de l'arbre AVL
    AVLNode* root = NULL;

    // Traitement du fichier CSV et remplissage de l'AVL
    printf("Analyse du fichier: %s\n", csv_file);
    root = process_input_csv(csv_file, root, mode);

    // Génération du fichier de sortie
    char output_file[256];
    // Utiliser un chemin absolu pour le fichier de sortie
    snprintf(output_file, sizeof(output_file), "../scripts/output_%s.csv", mode_str);
    printf("Génération du rapport: %s\n", output_file);
    generate_output_csv(output_file, root, mode);

    // Affichage des statistiques
    int node_count = 0;
    float total_value = 0.0f;

    // Fonction récursive locale pour compter et sommer
    void count_nodes(AVLNode* node) {
        if (node == NULL) return;

        node_count++;
        FactoryData* data = (FactoryData*)node->value;

        if (mode == 1) {
            total_value += data->capacity;
        } else if (mode == 2) {
            total_value += data->load_volume;
        } else if (mode == 3) {
            total_value += data->real_volume;
        }

        count_nodes(node->left);
        count_nodes(node->right);
    }

    count_nodes(root);

    printf("\nRésumé de l'analyse:\n");
    printf("- Nombre d'installations: %d\n", node_count);
    printf("- Total %s: %.2f\n",
           mode == 1 ? "des capacités" :
           mode == 2 ? "des volumes traités" :
           "des volumes réels", total_value);

    if (node_count > 0) {
        printf("- Moyenne par installation: %.2f\n", total_value / node_count);
    }

    // Libération de la mémoire
    avl_destroy(root, free_factory_data);
    if (need_free && csv_file) {
        free(csv_file);
    }

    printf("\nAnalysis complete.\n");
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
