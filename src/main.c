#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "csv_io.h"
#include "utils.h"
#include "histogram.h"
#include "histogram_modes.h"

// Déclaration de la fonction de conversion
char* convert_dat_to_csv(const char* dat_file);

// Fonction pour libérer la mémoire des données de l'usine
void free_factory_data(void* data) {
    free(data);
}

// Fonction pour calculer les pertes d'eau d'une usine
int calculate_leaks(const char* csv_file, const char* factory_id) {
    printf("Calcul des pertes pour l'usine: %s\n", factory_id);
    printf("Analyse du fichier: %s\n", csv_file); // Utilisation du paramètre csv_file

    // Cette fonction sera implémentée plus tard
    // Elle devra parcourir le réseau aval de l'usine et calculer les pertes

    // Pour l'instant, on simule juste un résultat
    double total_leaks = 0.0;
    FILE* history_file = fopen("../scripts/leaks_history.csv", "a");
    if (history_file) {
        // Vérifier si le fichier est vide pour ajouter un en-tête
        fseek(history_file, 0, SEEK_END);
        if (ftell(history_file) == 0) {
            fprintf(history_file, "Factory_ID;Total_Leaks_M3\n");
        }

        // Écrire une nouvelle entrée (pour l'instant avec des valeurs factices)
        fprintf(history_file, "%s;%.3f\n", factory_id, total_leaks);
        fclose(history_file);
    }

    printf("Volume total de pertes: %.3f millions de m³\n", total_leaks);
    return 0;
}

// Fonction pour générer un histogramme
int generate_histogram(const char* csv_file, HistogramMode mode) {
    printf("Génération d'un histogramme en mode: %s (%s)\n",
           histogram_mode_to_string(mode),
           get_histogram_mode_description(mode));
    // Initialisation de l'arbre AVL
    AVLNode* root = NULL;

    // Traitement du fichier CSV et remplissage de l'AVL
    printf("Analyse du fichier: %s\n", csv_file);

    // Utiliser l'ancien code pour traiter le fichier CSV
    // mais avec le nouveau mode d'histogramme
    int numeric_mode = mode; // Conversion simple pour la compatibilité
    root = process_input_csv(csv_file, root, numeric_mode);

    // Génération du fichier de sortie
    char output_file[256];
    snprintf(output_file, sizeof(output_file), "../scripts/%s.csv",
             get_output_filename_base(mode));

    printf("Génération du rapport: %s\n", output_file);
    generate_output_csv(output_file, root, numeric_mode);

    // Affichage des statistiques
    int node_count = 0;
    float total_value = 0.0f;

    // Fonction récursive locale pour compter et sommer
    void count_nodes(AVLNode* node) {
        if (node == NULL) return;

        node_count++;
        FactoryData* data = (FactoryData*)node->value;

        if (mode == HISTO_MODE_MAX) {
            total_value += data->capacity;
        } else if (mode == HISTO_MODE_SRC) {
            total_value += data->load_volume;
        } else if (mode == HISTO_MODE_REAL) {
            total_value += data->real_volume;
        }

        count_nodes(node->left);
        count_nodes(node->right);
    }

    count_nodes(root);

    printf("\nRésumé de l'analyse:\n");
    printf("- Nombre d'installations: %d\n", node_count);
    printf("- Total %s: %.2f\n",
           mode == HISTO_MODE_MAX ? "des capacités" :
           mode == HISTO_MODE_SRC ? "des volumes traités" :
           "des volumes réels", total_value);

    if (node_count > 0) {
        printf("- Moyenne par installation: %.2f\n", total_value / node_count);
    }

    // Libération de la mémoire
    avl_destroy(root, free_factory_data);

    return 0;
}

int main(int argc, char *argv[]) {
    // On attend 5 arguments : prog, input, output, mode, option
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <input.csv> <output.csv> <mode> <option>\n", argv[0]);
        return 1;
    }

    char *input_file = argv[1];
    char *output_file = argv[2]; // C'est ici qu'on récupère le fichier de sortie
    char *mode = argv[3];

    // Mesurer le temps d'exécution
    clock_t start_time = clock();

    // Vérification des arguments
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <fichier_donnees.dat> <commande> [options]\n", argv[0]);
        fprintf(stderr, "Commandes disponibles:\n");
        fprintf(stderr, "  histo <mode> - Génère un histogramme\n");
        fprintf(stderr, "    Modes: max, src, real, all\n");
        fprintf(stderr, "  leaks <id_usine> - Calcule les pertes pour une usine\n");
        return EXIT_FAILURE;
    }

    const char *input_file = argv[1];
    const char *command = argv[2];

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

    int result = EXIT_SUCCESS;

    // Traitement selon la commande
    if (strcmp(command, "histo") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Erreur: Mode d'histogramme manquant\n");
            fprintf(stderr, "Usage: %s <fichier> histo <mode>\n", argv[0]);
            result = EXIT_FAILURE;
        } else {
            const char *mode_str = argv[3];
            HistogramMode mode = string_to_histogram_mode(mode_str);

            if (!is_valid_histogram_mode(mode)) {
                fprintf(stderr, "Erreur: Mode d'histogramme non reconnu: %s\n", mode_str);
                fprintf(stderr, "Modes disponibles: max, src, real, all\n");
                result = EXIT_FAILURE;
            } else {
                result = generate_histogram(csv_file, mode);
            }
        }
    } else if (strcmp(command, "leaks") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Erreur: Identifiant d'usine manquant\n");
            fprintf(stderr, "Usage: %s <fichier> leaks <id_usine>\n", argv[0]);
            result = EXIT_FAILURE;
        } else {
            const char *factory_id = argv[3];
            result = calculate_leaks(csv_file, factory_id);
        }
    } else {
        fprintf(stderr, "Erreur: Commande non reconnue: %s\n", command);
        fprintf(stderr, "Commandes disponibles: histo, leaks\n");
        result = EXIT_FAILURE;
    }

    // Libération de la mémoire si nécessaire
    if (need_free && csv_file) {
        free(csv_file);
    }

    // Afficher le temps d'exécution
    clock_t end_time = clock();
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("\nTemps d'exécution: %.2f secondes\n", execution_time);

    return result;
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
