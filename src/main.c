#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "csv_io.h"
#include "utils.h"
#include "histogram.h"
#include "histogram_modes.h"

// Déclaration de la fonction de conversion (si elle existe dans utils.c ou make_csv.c)
// Sinon, assure-toi que le header correspondant est inclus.

void free_factory_data(void* data) {
    free(data);
}

// --- Modification ici : Ajout du paramètre output_csv_file ---
int generate_histogram(const char* input_csv_file, const char* output_csv_file, HistogramMode mode) {
    printf("Génération d'un histogramme en mode: %s (%s)\n",
           histogram_mode_to_string(mode),
           get_histogram_mode_description(mode));

    AVLNode* root = NULL;

    printf("Analyse du fichier: %s\n", input_csv_file);

    // Utilisation de l'ancien code pour traiter le fichier CSV
    int numeric_mode = mode;
    // Attention : process_input_csv dans csv_io.c a changé de signature dans tes fichiers précédents ?
    // Si process_input_csv renvoie void et prend (filename, output), ce n'est pas compatible ici.
    // Je suppose ici que tu utilises la version qui remplit l'AVL (basé sur ton code précédent).
    // Si ta fonction process_input_csv sert juste à nettoyer le CSV, il faut adapter.

    // HYPOTHÈSE : Tu as une fonction pour charger l'AVL.
    // Si ce n'est pas le cas, et que process_input_csv sert juste au nettoyage,
    // il faut appeler la fonction de chargement ici.
    // Pour l'instant, je garde ta logique :
    root = process_input_csv(input_csv_file, root, numeric_mode);

    printf("Génération du rapport: %s\n", output_csv_file);

    // Utilisation du fichier de sortie passé en argument
    generate_output_csv(output_csv_file, root, numeric_mode);

    // Affichage des statistiques (Code existant conservé)
    int node_count = 0;
    float total_value = 0.0f;

    void count_nodes(AVLNode* node) {
        if (node == NULL) return;
        node_count++;
        FactoryData* data = (FactoryData*)node->data;
        if (mode == HISTO_MODE_MAX) total_value += data->capacity;
        else if (mode == HISTO_MODE_SRC) total_value += data->load_volume;
        else if (mode == HISTO_MODE_REAL) total_value += data->real_volume;
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

    avl_destroy(root, free_factory_data);
    return 0;
}

int main(int argc, char *argv[]) {
    // 1. Vérification des arguments
    // On attend : prog <input.csv> <output.csv> <command> <option>
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <input.csv> <output.csv> <command> <option>\n", argv[0]);
        return 1;
    }

    // 2. Récupération des variables (Une seule fois !)
    char *input_file = argv[1];
    char *output_file = argv[2];
    char *command = argv[3]; // "histo" ou "leaks"
    char *option = argv[4];  // "all", "max", etc. ou ID usine

    printf("Starting C-WildWater analysis...\n");
    printf("Input: %s\nOutput: %s\nCommand: %s\nOption: %s\n", input_file, output_file, command, option);

    // 3. Aiguillage selon la commande
    if (strcmp(command, "histo") == 0) {
        HistogramMode mode;
        if (strcmp(option, "max") == 0) mode = HISTO_MODE_MAX;
        else if (strcmp(option, "src") == 0) mode = HISTO_MODE_SRC;
        else if (strcmp(option, "real") == 0) mode = HISTO_MODE_REAL;
        else if (strcmp(option, "all") == 0) mode = HISTO_MODE_ALL; // Si géré
        else {
            fprintf(stderr, "Mode inconnu: %s\n", option);
            return 1;
        }

        // Appel avec le fichier de sortie explicite
        generate_histogram(input_file, output_file, mode);

    } else if (strcmp(command, "leaks") == 0) {
        // calculate_leaks(input_file, option); // À décommenter si implémenté
        printf("Mode leaks non encore activé complètement.\n");
    } else {
        fprintf(stderr, "Commande inconnue: %s\n", command);
        return 1;
    }

    return 0;
}
