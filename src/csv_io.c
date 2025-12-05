#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "histogram.h" // Pour la struct FactoryData
#include "histogram_modes.h"

#define MAX_LINE_LENGTH 1024

AVLNode* process_input_csv(const char* input_filename, AVLNode* root, int mode) {
    FILE* fin = fopen(input_filename, "r");
    if (!fin) {
        perror("Erreur ouverture fichier entrée");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    // Sauter l'en-tête
    fgets(line, MAX_LINE_LENGTH, fin);

    while (fgets(line, MAX_LINE_LENGTH, fin)) {
        line[strcspn(line, "\r\n")] = 0; // Nettoyage saut de ligne

        // On utilise strtok pour découper la ligne par point-virgule
        char* token = strtok(line, ";");
        if (!token) continue;

        // 1. ID Station (récupéré comme chaîne)
        char* id_str = token;
        // 2. Amont (ignoré)
        strtok(NULL, ";");
        // 3. Aval (ignoré)
        strtok(NULL, ";");

        // 4. Volume (Capacity)
        char* vol_str = strtok(NULL, ";");
        double capacity = (vol_str && *vol_str != '-') ? atof(vol_str) : 0.0;

        // 5. Load / Fuite
        char* load_str = strtok(NULL, ";");
        double load = (load_str && *load_str != '-') ? atof(load_str) : 0.0;

        FactoryData* data = malloc(sizeof(FactoryData));
        if (data) {
            // Copie de la chaîne pour l'ID
            strncpy(data->id, id_str, sizeof(data->id) - 1);
            data->id[sizeof(data->id) - 1] = '\0'; // Sécurité
            data->capacity = capacity;
            data->load_volume = load;
            data->real_volume = capacity - load;

            // Insertion dans l'AVL
            root = insert_avl(root, data, mode);
        }
    }

    fclose(fin);
    return root;
}

// Fonction auxiliaire pour écrire l'AVL dans un CSV (Parcours en ordre)
void write_avl_to_csv(AVLNode* node, FILE* file, int mode) {
    if (node == NULL) return;

    // Parcours en ordre (gauche-racine-droite) pour un tri croissant
    write_avl_to_csv(node->left, file, mode);

    FactoryData* data = (FactoryData*)node->data;
    if (mode == HISTO_MODE_MAX) {
        fprintf(file, "%s;%.3f\n", data->id, data->capacity);
    } else if (mode == HISTO_MODE_SRC) {
        fprintf(file, "%s;%.3f\n", data->id, data->load_volume);
    } else if (mode == HISTO_MODE_REAL) {
        fprintf(file, "%s;%.3f\n", data->id, data->real_volume);
    } else if (mode == HISTO_MODE_ALL) {
        // Pour le mode "all", nous écrivons toutes les valeurs
        fprintf(file, "%s;%.3f;%.3f;%.3f\n", data->id, data->capacity, data->load_volume, data->real_volume);
    }

    write_avl_to_csv(node->right, file, mode);
}

void generate_output_csv(const char* filepath, AVLNode* root, int mode) {
    FILE* file = fopen(filepath, "w");
    if (!file) {
        perror("Error opening output file");
        return;
    }

    // En-tête adaptée selon le mode
    if (mode == HISTO_MODE_ALL) {
        fprintf(file, "Station;Capacity;Volume_Src;Volume_Real\n");
    } else {
        fprintf(file, "Station;Value\n");
    }

    write_avl_to_csv(root, file, mode);

    fclose(file);
}