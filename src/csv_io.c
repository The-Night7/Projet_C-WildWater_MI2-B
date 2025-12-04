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

void process_csv_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file"); // Affiche l'erreur système (ex: fichier introuvable)
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_LINE_LENGTH];  // Renommé pour éviter la confusion
    int line_count = 0;

    printf("Processing file: %s\n", filename);

    // Lecture ligne par ligne
    while (fgets(buffer, sizeof(buffer), file)) {
        line_count++;

        // Nettoyage de la ligne (suppression \n, \r)
        trim_whitespace(buffer);

        // Ignorer les lignes vides
        if (is_empty(buffer)) continue;

        // Découpage par point-virgule
        // Note: strtok n'est pas idéal pour les champs vides (ex: ";;"),
        // mais suffisant pour une première approche robuste si le format est respecté.
        char *token = strtok(buffer, ";");
        int field_index = 0;

        // Exemple d'affichage pour débogage
        // printf("Line %d: ", line_count);
        while (token != NULL) {
            // Ici, tu pourras stocker les données dans tes structures plus tard
            // printf("[%s] ", token);
            token = strtok(NULL, ";");
            field_index++;
        }
        // printf("\n");
    }

    printf("Total lines processed: %d\n", line_count);
    fclose(file);
}

/**
 * Lit le CSV d'entrée et remplit l'arbre AVL
 * @param filepath: chemin vers le fichier d'entrée
 * @param root: pointeur vers la racine AVL
 * @param mode: 1=max, 2=src, 3=real (pour savoir quelle colonne lire)
 */
AVLNode* process_input_csv(const char* filepath, AVLNode* root, int mode) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("Error opening input file");
        return root;
    }

    char line[BUFFER_SIZE];
    int lines_read = 0;
    int lines_matched = 0;
    int header_skipped = 0;

    // Afficher les 5 premières lignes pour débogage
    printf("Aperçu des 5 premières lignes du fichier:\n");
    int preview_count = 0;
    while (fgets(line, sizeof(line), file) && preview_count < 5) {
        printf("  %s", line);
        preview_count++;
    }

    // Revenir au début du fichier
    rewind(file);

    // Sauter l'en-tête si présent
    if (fgets(line, sizeof(line), file)) {
        if (strstr(line, "Station") || strstr(line, "Amont") || strstr(line, "Volume")) {
            header_skipped = 1;
        lines_read++;
                 } else {
            // Si ce n'est pas un en-tête, revenir au début de la ligne
            rewind(file);
        }
    }

    while (fgets(line, sizeof(line), file)) {
        lines_read++;

        // Colonnes: 1:Station, 2:Amont, 3:Aval, 4:Volume, 5:Fuite
        char* col1_station = get_field(line, 1); // Usine (qui traite)
        char* col2_amont = get_field(line, 2);   // Source/Amont
        char* col3_aval = get_field(line, 3);    // Destination/Aval
        char* col4_vol = get_field(line, 4);     // Volume
        char* col5_fuite = get_field(line, 5);   // Fuite

        float volume = parse_float(col4_vol);
        float fuite = parse_float(col5_fuite);

        // CAS 1: Histo Max (Capacité)
        // Recherche des lignes pour les capacités maximales des usines
        if (mode == HISTO_MODE_MAX) {
            // Chercher les lignes qui définissent la capacité des installations
            // Vérifier si col2_amont contient "Facility" ou "Plant" (installations)
            if (col2_amont && (strstr(col2_amont, "Facility") || strstr(col2_amont, "Plant"))) {

                // Vérifier si le nœud existe déjà
                AVLNode* node = avl_search(root, col2_amont);
                if (node) {
                    // Mise à jour des données existantes - prendre la valeur max
    FactoryData* data = (FactoryData*)node->value;
                    if (volume > data->capacity) {
                        data->capacity = volume;
                    }
                } else {
                    // Création d'une nouvelle structure avec initialisation complète
                    FactoryData* data = malloc(sizeof(FactoryData));
                    if (!data) {
                        perror("Memory allocation error");
                        continue;
    }
                    data->capacity = volume;
                    data->load_volume = 0;
                    data->real_volume = 0;
                    data->count = 1;

                    // Insérer dans AVL (Utilisant l'ID de Col 2)
                    root = avl_insert(root, col2_amont, data);
}
                lines_matched++;
            }
        }

        // CAS 2: Histo Src (Charge totale)
        else if (mode == HISTO_MODE_SRC) {
            // Chercher les lignes où une source alimente une installation
            if (col3_aval && (strstr(col3_aval, "Facility") || strstr(col3_aval, "Plant"))) {

                // Rechercher l'installation dans l'arbre
                AVLNode* node = avl_search(root, col3_aval);
                if (node) {
                    // Mise à jour des données existantes
                    FactoryData* data = (FactoryData*)node->value;
                    data->load_volume += volume;
                    data->count++;
                } else {
                    // Création d'une nouvelle structure avec initialisation complète
                    FactoryData* data = malloc(sizeof(FactoryData));
                    if (!data) {
                        perror("Memory allocation error");
                        continue;
                    }
                    data->capacity = 0;  // Sera rempli plus tard si nécessaire
                    data->load_volume = volume;
                    data->real_volume = 0;
                    data->count = 1;

                    root = avl_insert(root, col3_aval, data);
                }
                lines_matched++;
            }
        }

        // CAS 3: Histo Real (Volume réel consommé)
        else if (mode == HISTO_MODE_REAL) {
            // Identifier les lignes pertinentes pour le mode "real"
            // Lignes où une installation traite de l'eau
            if (col1_station && (strstr(col1_station, "Facility") || strstr(col1_station, "Plant"))) {

                AVLNode* node = avl_search(root, col1_station);
                if (node) {
    FactoryData* data = (FactoryData*)node->value;
                    data->real_volume += volume - (volume * fuite / 100.0);  // Volume réel = volume traité - fuites
                    data->count++;
                } else {
                    FactoryData* data = malloc(sizeof(FactoryData));
                    if (!data) {
                        perror("Memory allocation error");
                        continue;
                    }
                    data->capacity = 0;
                    data->load_volume = 0;
                    data->real_volume = volume - (volume * fuite / 100.0);
                    data->count = 1;

                    root = avl_insert(root, col1_station, data);
                }
                lines_matched++;
            }
        }

        // CAS 4: Histo All (tous les modes combinés - bonus)
        else if (mode == HISTO_MODE_ALL) {
            // Pour le mode "all", nous devons collecter toutes les données
            // Traiter les données pour la capacité maximale
            if (col2_amont && (strstr(col2_amont, "Facility") || strstr(col2_amont, "Plant"))) {
                AVLNode* node = avl_search(root, col2_amont);
                if (node) {
                    FactoryData* data = (FactoryData*)node->value;
                    if (volume > data->capacity) {
                        data->capacity = volume;
                    }
                } else {
                    FactoryData* data = malloc(sizeof(FactoryData));
                    if (!data) {
                        perror("Memory allocation error");
                        continue;
                    }
                    data->capacity = volume;
                    data->load_volume = 0;
                    data->real_volume = 0;
                    data->count = 1;
                    root = avl_insert(root, col2_amont, data);
                }
                lines_matched++;
            }

            // Traiter les données pour le volume capté
            if (col3_aval && (strstr(col3_aval, "Facility") || strstr(col3_aval, "Plant"))) {
                AVLNode* node = avl_search(root, col3_aval);
                if (node) {
                    FactoryData* data = (FactoryData*)node->value;
                    data->load_volume += volume;
                } else {
                    FactoryData* data = malloc(sizeof(FactoryData));
                    if (!data) {
                        perror("Memory allocation error");
                        continue;
                    }
                    data->capacity = 0;
                    data->load_volume = volume;
                    data->real_volume = 0;
                    data->count = 1;
                    root = avl_insert(root, col3_aval, data);
                }
                lines_matched++;
            }

            // Traiter les données pour le volume réel
            if (col1_station && (strstr(col1_station, "Facility") || strstr(col1_station, "Plant"))) {
                AVLNode* node = avl_search(root, col1_station);
                if (node) {
                    FactoryData* data = (FactoryData*)node->value;
                    data->real_volume += volume - (volume * fuite / 100.0);
                } else {
                    FactoryData* data = malloc(sizeof(FactoryData));
                    if (!data) {
                        perror("Memory allocation error");
                        continue;
                    }
                    data->capacity = 0;
                    data->load_volume = 0;
                    data->real_volume = volume - (volume * fuite / 100.0);
                    data->count = 1;
                    root = avl_insert(root, col1_station, data);
                }
                lines_matched++;
            }
        }
    }

    printf("Lignes lues: %d, lignes correspondant aux critères du mode %d: %d\n",
           lines_read, mode, lines_matched);
    fclose(file);
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
