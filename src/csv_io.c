#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "histogram.h" // Pour la struct FactoryData

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
    
    while (fgets(line, sizeof(line), file)) {
        // Colonnes: 1:Station, 2:Amont, 3:Aval, 4:Volume, 5:Fuite
        // Remarque: Le PDF indique que les colonnes dépendent du type de ligne.
        // Nous devons être prudents. Extrayons tous les champs potentiellement utiles.

        char* col1_station = get_field(line, 1); // Usine (qui traite)
        char* col2_amont = get_field(line, 2);   // Source/Amont
        char* col4_vol = get_field(line, 4);     // Volume
        
        float volume = parse_float(col4_vol);

        // LOGIQUE SELON LE MODE

        // CAS 1: Histo Max (Capacité)
        // Recherche des lignes: "-;Facility complex #...;-;4749292;-"
        // Col 2 est l'ID de l'usine, Col 4 est la capacité. Col 1 est vide ("-").
        if (mode == 1) {
            if (col1_station && strcmp(col1_station, "-") == 0 && 
                col2_amont && strstr(col2_amont, "Facility")) {
                
                // Vérifier si le nœud existe, sinon le créer
                FactoryData* data = malloc(sizeof(FactoryData));
                data->capacity = volume;
                data->load_volume = 0;
                
                // Insérer dans AVL (Utilisant l'ID de Col 2)
                root = avl_insert(root, col2_amont, data);
            }
        }
        
        // CAS 2: Histo Src (Charge totale)
        // Recherche des lignes: "-;Spring...;Facility...;20892;..."
        // Col 2 est la Source, Col 3 est l'Usine (Attendez, votre PDF dit que Col 1 est généralement l'Usine?)
        // Relisons attentivement le PDF:
        // "SOURCE -> USINE: -;Spring...;Facility...;Vol;..." -> Col 3 est l'Usine?
        // Le PDF dit: "Col 1 contient l'ID unique de l'usine qui a traité l'eau... SAUF pour les lignes Source->Usine?"
        // En fait, le PDF dit pour Source->Usine: "Col 1 n'est pas utilisée (-)". L'usine est dans Col 3.
        else if (mode == 2) {
             char* col3_aval = get_field(line, 3);
             
             if (col1_station && strcmp(col1_station, "-") == 0 && 
                 col3_aval && strstr(col3_aval, "Facility")) {
                 
                 // Nous devons METTRE À JOUR le nœud pour cette usine (col3_aval)
                 AVLNode* node = avl_search(root, col3_aval);
                 if (node) {
                     FactoryData* data = (FactoryData*)node->value;
                     data->load_volume += volume;
                 } else {
                     FactoryData* data = malloc(sizeof(FactoryData));
                     memset(data, 0, sizeof(FactoryData));
                     data->load_volume = volume;
                     root = avl_insert(root, col3_aval, data);
                 }
             }
        }
    }

    fclose(file);
    return root;
}

// Fonction auxiliaire pour écrire l'AVL dans un CSV (Parcours inverse en ordre)
void write_avl_to_csv(AVLNode* node, FILE* file, int mode) {
    if (node == NULL) return;

    // Ordre inverse: Droite -> Racine -> Gauche (pour ordre décroissant si demandé,
    // sinon Gauche -> Racine -> Droite pour alphabétique)
    // Le sujet demande souvent "alphabétique" ou "numérique".
    // Si c'est alphabétique : Left -> Root -> Right.
    
    write_avl_to_csv(node->left, file, mode);

    FactoryData* data = (FactoryData*)node->value;
    if (mode == 1) {
        fprintf(file, "%s;%.3f\n", node->key, data->capacity);
    } else if (mode == 2) {
        fprintf(file, "%s;%.3f\n", node->key, data->load_volume);
    }

    write_avl_to_csv(node->right, file, mode);
}

void generate_output_csv(const char* filepath, AVLNode* root, int mode) {
    FILE* file = fopen(filepath, "w");
    if (!file) {
        perror("Error opening output file");
        return;
    }
    
    // En-tête requise par le sujet?
    fprintf(file, "Station;Value\n");

    write_avl_to_csv(root, file, mode);
    
    fclose(file);
}
