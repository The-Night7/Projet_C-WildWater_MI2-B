#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Définition d'une valeur pour les données manquantes ('-')
#define MISSING_DATA -1.0f

/**
 * Analyse une ligne du fichier CSV et extrait les informations demandées.
 * * @param line : La ligne brute lue depuis le fichier (ex: "Plant;Source A;Usine B;1000;0.5")
 * @param id_amont_out : Buffer pour stocker l'ID amont (colonne 2)
 * @param id_aval_out : Buffer pour stocker l'ID aval (colonne 3)
 * @param volume_out : Pointeur pour le volume (colonne 4)
 * @param fuite_out : Pointeur pour le pourcentage (colonne 5)
 * @return : 0 si succès, 1 si erreur de format.
 */
int parse_line(char *line, char *id_amont_out, char *id_aval_out, float *volume_out, float *fuite_out) {
    char *token;
    char buffer[1024]; // Copie de travail pour ne pas altérer la ligne originale si besoin
    
    // Nettoyage du saut de ligne final éventuel
    line[strcspn(line, "\r\n")] = 0;
    strncpy(buffer, line, 1024);

    // --- Colonne 1 : Usine (Info contextuelle, souvent ignorée pour le tronçon lui-même) ---
    token = strtok(buffer, ";");
    if (!token) return 1; 

    // --- Colonne 2 : ID Amont [cite: 61] ---
    token = strtok(NULL, ";");
    if (!token) return 1;
    strcpy(id_amont_out, token);

    // --- Colonne 3 : ID Aval [cite: 62] ---
    token = strtok(NULL, ";");
    if (!token) return 1;
    strcpy(id_aval_out, token);

    // --- Colonne 4 : Volume [cite: 63] ---
    token = strtok(NULL, ";");
    if (!token) return 1;
    // Gestion du tiret '-' 
    if (strcmp(token, "-") == 0) {
        *volume_out = MISSING_DATA;
    } else {
        *volume_out = strtof(token, NULL);
    }

    // --- Colonne 5 : Pourcentage de fuites [cite: 64] ---
    token = strtok(NULL, ";");
    if (!token) return 1;
    // Gestion du tiret '-'
    if (strcmp(token, "-") == 0) {
        *fuite_out = MISSING_DATA;
    } else {
        *fuite_out = strtof(token, NULL);
    }

    return 0; // Succès
}



void charger_donnees(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erreur ouverture fichier");
        exit(1); // Le code erreur doit être strictement positif [cite: 212]
    }

    char line[1024];
    char amont[256], aval[256];
    float vol, fuite;

    // Lecture ligne par ligne
    while (fgets(line, sizeof(line), file)) {
        
        // On parse la ligne
        if (parse_line(line, amont, aval, &vol, &fuite) == 0) {
            
            // ICI : Insérez votre logique de stockage en mémoire.
            // Le sujet suggère d'utiliser des AVL pour les recherches rapides[cite: 207].
            
            /* Exemple fictif :
            if (vol != MISSING_DATA) {
                ajouter_dans_avl(mon_arbre, amont, vol);
            }
            ajouter_liaison(amont, aval, fuite);
            */
            
            // Debug simple pour vérifier que ça marche
            // printf("Tronçon: %s -> %s | Vol: %.2f | Fuite: %.2f%%\n", amont, aval, vol, fuite);
        }
    }

    fclose(file);
}
