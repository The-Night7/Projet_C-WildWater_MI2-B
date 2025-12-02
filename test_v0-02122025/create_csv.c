#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MISSING_DATA -1.0f

// --- FONCTION DE PARSING (Celle discutée précédemment) ---
int parse_line(char *line, char *usine_ref, char *id_amont, char *id_aval, float *vol, float *fuite) {
    char buffer[MAX_LINE];
    char *token;

    // Copie pour ne pas détruire la ligne originale (utile pour l'affichage debug)
    strncpy(buffer, line, MAX_LINE);
    buffer[strcspn(buffer, "\r\n")] = 0; // Enlever le \n

    // 1. Usine (Col 1)
    token = strtok(buffer, ";");
    if (!token) return 1;
    strcpy(usine_ref, (strcmp(token, "-") == 0) ? "N/A" : token);

    // 2. Amont (Col 2) [cite: 61]
    token = strtok(NULL, ";");
    if (!token) return 1;
    strcpy(id_amont, token);

    // 3. Aval (Col 3) [cite: 62]
    token = strtok(NULL, ";");
    if (!token) return 1;
    strcpy(id_aval, (strcmp(token, "-") == 0) ? "N/A" : token);

    // 4. Volume (Col 4) [cite: 63]
    token = strtok(NULL, ";");
    if (!token) return 1;
    *vol = (strcmp(token, "-") == 0) ? MISSING_DATA : strtof(token, NULL);

    // 5. Fuite (Col 5) [cite: 64]
    token = strtok(NULL, ";");
    if (!token) return 1;
    *fuite = (strcmp(token, "-") == 0) ? MISSING_DATA : strtof(token, NULL);

    return 0; // Succès
}

// --- MAIN DE TEST ---
int main() {
    const char *filename = "mini_test.dat";

    // ---------------------------------------------------------
    // ETAPE 1 : Création d'un fichier de test (Mock Data)
    // On reproduit les exemples du PDF pour vérifier le parsing
    // ---------------------------------------------------------
    printf("Creation du fichier de test '%s'...\n", filename);
    FILE *f_create = fopen(filename, "w");
    if (!f_create) { perror("Erreur creation"); return 1; }

    // Cas 1 : Source -> Usine (Volume présent, Pas de fuite indiquée ici) [cite: 71-73]
    fprintf(f_create, "-;Spring #MQ001991L;Facility complex #RH400057F;20892;0.997\n");
    
    // Cas 2 : Définition Usine (Capacité présente, Pas d'aval) [cite: 79-82]
    fprintf(f_create, "-;Facility complex #RH400057F;-;4749292;-\n");

    // Cas 3 : Usine -> Stockage (Pas de volume, Fuite présente) [cite: 90-93]
    fprintf(f_create, "-;Facility complex #RH400057F;Storage #13178;-;3.777\n");

    // Cas 4 : Stockage -> Jonction (Usine ref en Col 1, Fuite présente) [cite: 100-103]
    fprintf(f_create, "Facility complex #RH400057F;Storage #13178;Junction #TM12995S;-;3.308\n");

    fclose(f_create);
    printf("Fichier cree avec succes.\n\n");

    // ---------------------------------------------------------
    // ETAPE 2 : Lecture et Vérification
    // ---------------------------------------------------------
    printf("--- DEBUT DU TEST DE LECTURE ---\n");
    
    FILE *file = fopen(filename, "r");
    if (!file) { perror("Erreur lecture"); return 1; }

    char line[MAX_LINE];
    char usine[50], amont[50], aval[50];
    float volume, fuite;
    int ligne_num = 1;

    while (fgets(line, sizeof(line), file)) {
        // Affichage de la ligne brute pour comparaison
        printf("Ligne %d brute : %s", ligne_num, line); 
        // Note: fgets garde le \n, donc pas besoin d'en ajouter un dans le printf

        if (parse_line(line, usine, amont, aval, &volume, &fuite) == 0) {
            printf("   -> PARSING OK :\n");
            printf("      [Col 1] Usine Ref : %s\n", usine);
            printf("      [Col 2] Amont     : %s\n", amont);
            printf("      [Col 3] Aval      : %s\n", aval);
            
            // Affichage intelligent du Volume
            printf("      [Col 4] Volume    : ");
            if (volume == MISSING_DATA) printf("Non defini (-)\n");
            else printf("%.2f m3\n", volume);

            // Affichage intelligent de la Fuite
            printf("      [Col 5] Fuite     : ");
            if (fuite == MISSING_DATA) printf("Non defini (-)\n");
            else printf("%.3f %%\n", fuite);
            
        } else {
            printf("   -> ERREUR de format sur cette ligne.\n");
        }
        printf("---------------------------------------------------\n");
        ligne_num++;
    }

    fclose(file);
    printf("Fin du test v0.\n");
    return 0;
}
