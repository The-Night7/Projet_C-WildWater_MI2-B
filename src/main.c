#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

// --- Fonction récursive de calcul des fuites (DFS) ---
// Retourne le volume TOTAL perdu en aval de ce noeud
double solve_leaks(Station* node, double input_vol) {
    if (!node || node->nb_children == 0) return 0.0;

    double total_loss = 0.0;
    // Répartition équitable du flux [cite: 167]
    double vol_per_pipe = input_vol / node->nb_children;

    AdjNode* curr = node->children;
    while (curr) {
        // Calcul perte sur ce tronçon
        double pipe_loss = 0;
        if (curr->leak_perc > 0) {
            pipe_loss = vol_per_pipe * (curr->leak_perc / 100.0);
        }
        
        // Ce qui arrive réellement à l'enfant
        double vol_arrived = vol_per_pipe - pipe_loss;

        // Somme: Perte locale + Pertes des enfants
        total_loss += pipe_loss + solve_leaks(curr->target, vol_arrived);

        curr = curr->next;
    }
    return total_loss;
}

// --- Main ---
int main(int argc, char** argv) {
    if (argc != 3) return 1; // [cite: 214] code retour > 0 si erreur

    FILE* file = fopen(argv[1], "r");
    if (!file) return 2;

    char* arg_mode = argv[2];
    int mode_histo = 0; // 1=max, 2=src, 3=real
    int mode_leaks = 0; 

    if (strcmp(arg_mode, "max") == 0) mode_histo = 1;
    else if (strcmp(arg_mode, "src") == 0) mode_histo = 2;
    else if (strcmp(arg_mode, "real") == 0) mode_histo = 3;
    else mode_leaks = 1; // Sinon c'est un ID d'usine

    Station* root = NULL;
    char line[1024];

    // --- Lecture du fichier ---
    // --- Lecture du fichier ---
    long line_count = 0; // Compteur de lignes

    while (fgets(line, sizeof(line), file)) {
        // --- AJOUT DEBUG : Affichage tous les 200 000 lignes ---
        line_count++;
        if (line_count % 200000 == 0) {
            // \r permet de revenir au début de la ligne sans sauter de ligne
            fprintf(stderr, "Traitement en cours : %ld lignes lues...\r", line_count);
            fflush(stderr); // Force l'affichage immédiat
        }
        // -------------------------------------------------------

        // Nettoyage sauts de ligne
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) < 2) continue;

        // Parsing manuel
        char* cols[5] = {NULL};
        char* p = line;
        int c = 0;
        
        cols[c++] = p;
        while (*p && c < 5) {
            if (*p == ';') {
                *p = '\0';
                cols[c++] = p + 1;
            }
            p++;
        }

        // Nettoyage des "-"
        for(int i=0; i<5; i++) {
            if (cols[i] && (strcmp(cols[i], "-") == 0 || strlen(cols[i]) == 0)) {
                cols[i] = NULL;
            }
        }

        // --- Logique LEAKS ---
        if (mode_leaks) {
            if (cols[1]) {
                if (!find_station(root, cols[1])) 
                    root = insert_station(root, cols[1], 0, 0, 0);
            }
            if (cols[2]) {
                if (!find_station(root, cols[2])) 
                    root = insert_station(root, cols[2], 0, 0, 0);
            }

            if (cols[1] && cols[2]) {
                Station* pa = find_station(root, cols[1]);
                Station* ch = find_station(root, cols[2]);
                double leak = (cols[4]) ? atof(cols[4]) : 0.0;
                add_connection(pa, ch, leak);
            }

            // Capacité
            if (cols[1] && !cols[2] && cols[3]) {
                Station* s = find_station(root, cols[1]);
                if (s) s->capacity = atol(cols[3]);
            }
        }
        // --- Logique HISTO ---
        else {
            if (mode_histo == 1 && cols[1] && !cols[2] && cols[3]) {
                if (strstr(cols[1], "Plant"))
                    root = insert_station(root, cols[1], atol(cols[3]), 0, 0);
            }
            else if ((mode_histo == 2 || mode_histo == 3) && cols[2] && strstr(cols[2], "Plant")) {
                 if (cols[3]) {
                     long vol = atol(cols[3]);
                     long reel = vol;
                     if (mode_histo == 3 && cols[4]) {
                         double p = atof(cols[4]);
                         reel = (long)(vol * (1.0 - (p/100.0)));
                     }
                     root = insert_station(root, cols[2], 0, vol, reel);
                 }
            }
        }
    }
    // Nettoyage visuel fin de chargement
    fprintf(stderr, "Chargement terminé : %ld lignes. Calcul en cours...\n", line_count);
    
    fclose(file);

    // --- Sortie des résultats ---
    if (mode_leaks) {
        Station* start = find_station(root, arg_mode);
        if (!start) {
            // Usine introuvable [cite: 168] => retourne 0 (le script gère le -1)
            printf("0\n"); 
        } else {
            // Calculer les fuites sur la base de la capacité de l'usine
            double leaks = solve_leaks(start, (double)start->capacity);
            
            // Le sujet demande en M.m3 (Millions m3). 
            // Si capacity est en m3 : / 1,000,000
            // Si capacity est en k.m3 : / 1,000
            // Supposons k.m3 comme le reste du fichier :
            printf("%f\n", leaks / 1000.0); 
        }
    } else {
        // Mode Histo (max, src, real)
        char mode_str[10];
        if (mode_histo == 1) strcpy(mode_str, "max");
        if (mode_histo == 2) strcpy(mode_str, "src");
        if (mode_histo == 3) strcpy(mode_str, "real");
        write_csv(root, stdout, mode_str);
    }

    free_tree(root);
    return 0;
}