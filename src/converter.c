#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Ce programme lit un fichier .dat (séparateur ;) et écrit un CSV propre (séparateur ,)
// Il remplace les tirets '-' par '0' ou des chaines vides pour faciliter le parsing suivant.

void parse_and_write(char *line, FILE *out) {
    // Supprimer le saut de ligne final
    line[strcspn(line, "\n")] = 0;
    line[strcspn(line, "\r")] = 0; // Sécurité Windows

    char col1[64] = "0";
    char col2[64] = "0";
    char col3[64] = "0";
    char col4[64] = "0";
    char col5[64] = "0";

    // Lecture formatée : %[^;] lit tout jusqu'au point-virgule
    // On utilise sscanf pour extraire les 5 champs.
    // Si un champ est vide (ex: ;;), sscanf peut être capricieux, 
    // mais dans ce projet les champs vides sont marqués par '-' ou sont explicitement vides.
    
    // Astuce : On va découper manuellement avec strtok pour être plus robuste
    // car sscanf gère mal les champs vides consécutifs
    
    char *token;
    int i = 0;
    
    // Copie de travail
    char tmp[512];
    strncpy(tmp, line, 511);
    tmp[511] = '\0';

    // On remplace les ;; par ;-; pour aider strtok si besoin (cas rare ici)
    // Pour ce projet, on suppose le format respecté : Val;Val;Val;Val;Val
    
    token = strtok(tmp, ";");
    if(token) { strcpy(col1, token); token = strtok(NULL, ";"); }
    if(token) { strcpy(col2, token); token = strtok(NULL, ";"); }
    if(token) { strcpy(col3, token); token = strtok(NULL, ";"); }
    if(token) { strcpy(col4, token); token = strtok(NULL, ";"); }
    if(token) { strcpy(col5, token); }

    // Remplacement des tirets isolés par 0
    if(strcmp(col1, "-") == 0) strcpy(col1, "0");
    if(strcmp(col2, "-") == 0) strcpy(col2, "0");
    if(strcmp(col3, "-") == 0) strcpy(col3, "0");
    if(strcmp(col4, "-") == 0) strcpy(col4, "0");
    if(strcmp(col5, "-") == 0) strcpy(col5, "0");

    fprintf(out, "%s,%s,%s,%s,%s\n", col1, col2, col3, col4, col5);
}

int main(int argc, char *argv[]) {
    if (argc != 3) return 1;

    FILE *in = fopen(argv[1], "r");
    if (!in) return 2;

    FILE *out = fopen(argv[2], "w");
    if (!out) { fclose(in); return 3; }

    char line[1024];
    while (fgets(line, sizeof(line), in)) {
        if (strlen(line) > 2) { // Ignorer lignes vides
            parse_and_write(line, out);
        }
    }

    fclose(in);
    fclose(out);
    return 0;
}