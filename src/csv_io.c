#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "histogram.h" // Pour la struct FactoryData
#include "histogram_modes.h"

#define BUFFER_SIZE 1024
#define MAX_LINE_LENGTH 1024  // Définition de la constante manquante

// Fonction auxiliaire pour analyser en toute sécurité un float à partir d'une chaîne, gérant "-" ou vide
float parse_float(char* str) {
    if (!str || strcmp(str, "-") == 0 || strlen(str) == 0) return 0.0f;
    return strtof(str, NULL);
}

// Fonction auxiliaire pour obtenir un champ d'une ligne séparée par des points-virgules (non destructive si possible, mais ici nous modifions la copie)
char* get_field(char* line, int index) {
    static char buffer[256];
    int col = 1;
    char* ptr = line;
    char* start = line;

    while (*ptr) {
        if (*ptr == ';') {
            if (col == index) {
                int len = ptr - start;
                if (len >= 256) len = 255;
                strncpy(buffer, start, len);
                buffer[len] = '\0';
                return buffer;
            }
            col++;
            start = ptr + 1;
        }
        ptr++;
    }
    // Gestion de la dernière colonne
    if (col == index) {
        strncpy(buffer, start, 255);
        buffer[255] = '\0';
        // Suppression du saut de ligne potentiel
        buffer[strcspn(buffer, "\r\n")] = 0;
        return buffer;
    }
    return NULL;
}

void get_field_safe(char* line, int index, char* dest, size_t dest_size) {
    // On travaille sur une copie pour ne pas détruire la ligne originale si besoin
    char temp_line[MAX_LINE_LENGTH];
    strncpy(temp_line, line, MAX_LINE_LENGTH);
    temp_line[MAX_LINE_LENGTH - 1] = '\0';

    char* token = strtok(temp_line, ";");
    int current_col = 1;

    while (token != NULL) {
        if (current_col == index) {
            strncpy(dest, token, dest_size);
            dest[dest_size - 1] = '\0';
            return;
        }
        token = strtok(NULL, ";");
        current_col++;
    }
    // Si non trouvé, chaîne vide
    dest[0] = '\0';
}

void process_csv_file(const char *filename) {
    FILE *fin = fopen(filename, "r");
    if (fin == NULL) {
        perror("Error opening file"); // Affiche l'erreur système (ex: fichier introuvable)
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    long line_count = 0;
    long written_count = 0;

    printf("Processing file: %s\n", filename);

    // 1. Sauter l'en-tête (Station;Amont;Aval;Volume;Fuite)
    if (!fgets(line, MAX_LINE_LENGTH, fin)) {
        fclose(fin);
        return;
    }
    // 2. Lecture ligne par ligne
    while (fgets(line, MAX_LINE_LENGTH, fin)) {
        line_count++;

        // Suppression du saut de ligne final
        line[strcspn(line, "\r\n")] = 0;

        // Copie de la ligne pour ne pas détruire l'original si besoin de debug,
        // mais ici on travaille directement sur 'line' pour l'efficacité.

        // Parsing strict sur le point-virgule
        char* station = strtok(line, ";");
        char* amont   = strtok(NULL, ";");
        char* aval    = strtok(NULL, ";");
        char* volume  = strtok(NULL, ";");
        char* fuite   = strtok(NULL, ";");

        // Si la ligne est complète (5 tokens trouvés)
        if (station && amont && aval && volume && fuite) {
            // Ici, tu pourras stocker les données dans tes structures plus tard
            written_count++;
        }
    }

    printf("Total lines processed: %ld\n", line_count);
    printf("Total valid lines: %ld\n", written_count);
    fclose(fin);
}

/**
 * Lit le CSV d'entrée et remplit l'arbre AVL
 * @param filepath: chemin vers le fichier d'entrée
 * @param root: pointeur vers la racine AVL
 * @param mode: 1=max, 2=src, 3=real (pour savoir quelle colonne lire)
 */
// Fonction pour charger les données du CSV dans l'AVL
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
        line[strcspn(line, "\r\n")] = 0; // Nettoyage fin de ligne

        // Parsing (Station;Amont;Aval;Volume;Fuite)
        char* token = strtok(line, ";");
        if (!token) continue;

        // ID Station
        int station_id = atoi(token); // Ou garder en char* selon ta struct

        // On saute Amont (2) et Aval (3) pour l'instant si pas utilisés
        strtok(NULL, ";");
        strtok(NULL, ";");

        // Volume (4)
        char* vol_str = strtok(NULL, ";");
        double capacity = vol_str ? atof(vol_str) : 0.0;

        // Fuite (5) - ou autre selon tes colonnes
        char* leak_str = strtok(NULL, ";");
        double load = leak_str ? atof(leak_str) : 0.0;

        // Création de la structure de données
        FactoryData* data = malloc(sizeof(FactoryData));
        if (data) {
            data->id = station_id;
            data->capacity = capacity;
            data->load_volume = load;
            data->real_volume = capacity - load; // Exemple de calcul

            // Insertion dans l'AVL
            root = avl_insert(root, data, mode);
        }
    }

    fclose(fin);
    return root;
}

// Fonction auxiliaire pour écrire l'AVL dans un CSV (Parcours inverse en ordre)
void write_avl_to_csv(AVLNode* node, FILE* file, int mode) {
    if (node == NULL) return;

    // Parcours en ordre inverse (droite-racine-gauche) pour un tri alphabétique inverse
    write_avl_to_csv(node->right, file, mode);

    FactoryData* data = (FactoryData*)node->value;
    if (mode == HISTO_MODE_MAX) {
        fprintf(file, "%s;%.3f\n", node->key, data->capacity);
    } else if (mode == HISTO_MODE_SRC) {
        fprintf(file, "%s;%.3f\n", node->key, data->load_volume);
    } else if (mode == HISTO_MODE_REAL) {
        fprintf(file, "%s;%.3f\n", node->key, data->real_volume);
    } else if (mode == HISTO_MODE_ALL) {
        // Pour le mode "all", nous écrivons toutes les valeurs
        fprintf(file, "%s;%.3f;%.3f;%.3f\n", node->key, data->capacity, data->load_volume, data->real_volume);
    }

    write_avl_to_csv(node->left, file, mode);
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