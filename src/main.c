/*
 * main.c
 *
 * Version corrigée et optimisée :
 * 1. Ajout du nettoyage des espaces (TRIM) pour éviter les erreurs "Introuvable".
 * 2. Optimisation du mode LEAKS (1 recherche au lieu de 4).
 * 3. Buffer I/O augmenté pour WSL.
 * 4. Ajout du mode "all" pour le Bonus 1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // Nécessaire pour isspace()
#include "avl.h"

// -----------------------------------------------------------------------------
//  Fonction récursive de calcul des fuites (DFS)
// -----------------------------------------------------------------------------
static double solve_leaks(Station* node, double input_vol) {
    if (!node || node->nb_children == 0) {
        return 0.0;
    }
    double total_loss = 0.0;
    double vol_per_pipe = input_vol / node->nb_children;
    AdjNode* curr = node->children;
    while (curr) {
        // Perte sur ce tronçon
        double pipe_loss = 0.0;
        if (curr->leak_perc > 0) {
            pipe_loss = vol_per_pipe * (curr->leak_perc / 100.0);
        }
        // Ce qui arrive réellement à l’enfant
        double vol_arrived = vol_per_pipe - pipe_loss;
        // Somme : perte locale + pertes des enfants
        total_loss += pipe_loss + solve_leaks(curr->target, vol_arrived);
        curr = curr->next;
    }
    return total_loss;
}

// -----------------------------------------------------------------------------
//  Main
// -----------------------------------------------------------------------------
int main(int argc, char** argv) {
    if (argc != 3) {
        return 1;
    }
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        return 2;
    }

    // --- OPTIMISATION I/O ---
    const size_t BUF_SIZE = 16 * 1024 * 1024; // 16 Mo
    char* big_buffer = malloc(BUF_SIZE);
    if (big_buffer) setvbuf(file, big_buffer, _IOFBF, BUF_SIZE);
    // ------------------------

    char* arg_mode = argv[2];
    int mode_histo = 0; // 1 = max, 2 = src, 3 = real, 4 = all
    int mode_leaks = 0;
    
    if (strcmp(arg_mode, "max") == 0) {
        mode_histo = 1;
    } else if (strcmp(arg_mode, "src") == 0) {
        mode_histo = 2;
    } else if (strcmp(arg_mode, "real") == 0) {
        mode_histo = 3;
    } else if (strcmp(arg_mode, "all") == 0) { // AJOUT BONUS
        mode_histo = 4;
    } else {
        mode_leaks = 1;
    }

    Station* root = NULL;
    char line[1024];
    long line_count = 0;

    while (fgets(line, sizeof(line), file)) {
        line_count++;
        // Affichage progression (stderr)
        if (line_count % 200000 == 0) {
            fprintf(stderr, "Lignes traitees : %ld...\r", line_count);
            fflush(stderr);
        }

        // Nettoyage fin de ligne
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) < 2) continue;

        // Découpage
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

        // --- CORRECTION CRITIQUE : Nettoyage des espaces (TRIM) ---
        for (int i = 0; i < 5; i++) {
            if (cols[i]) {
                // Trim début
                while (*cols[i] == ' ') cols[i]++;
                // Trim fin
                char* end = cols[i] + strlen(cols[i]) - 1;
                while (end > cols[i] && *end == ' ') {
                    *end = '\0';
                    end--;
                }
                // Vérif vide ou tiret
                if (strcmp(cols[i], "-") == 0 || strlen(cols[i]) == 0) {
                    cols[i] = NULL;
                }
            }
        }
        // ----------------------------------------------------------

        if (mode_leaks) {
            // --- OPTIMISATION : Une seule recherche par noeud ---
            Station* pa = NULL;
            Station* ch = NULL;

            // 1. Gérer le parent (émetteur)
            if (cols[1]) {
                pa = find_station(root, cols[1]);
                if (!pa) {
                    root = insert_station(root, cols[1], 0, 0, 0);
                    pa = find_station(root, cols[1]); // On récupère l'adresse stable
                }
            }

            // 2. Gérer l'enfant (récepteur)
            if (cols[2]) {
                ch = find_station(root, cols[2]);
                if (!ch) {
                    root = insert_station(root, cols[2], 0, 0, 0);
                    ch = find_station(root, cols[2]); // On récupère l'adresse stable
                }
            }

            // 3. Créer la connexion
            if (pa && ch) {
                double leak = (cols[4]) ? atof(cols[4]) : 0.0;
                add_connection(pa, ch, leak);

                // Calcul du volume réel entrant pour l'usine cible
                if (cols[3] && strcmp(ch->name, arg_mode) == 0) {
                    double vol = atof(cols[3]);
                    ch->real_qty += (long)(vol * (1.0 - leak/100.0));
                }
            }

            // 4. Mettre à jour la capacité (si c'est la ligne de définition de l'usine)
            if (pa && !cols[2] && cols[3]) {
                pa->capacity = atol(cols[3]);
            }
            // ----------------------------------------------------

        } else {
            // Mode Histogramme : agrégation

            // Cas 1 : Définition de l'usine (Capacité)
            // Concerne les modes : "max" (1) et "all" (4)
            if ((mode_histo == 1 || mode_histo == 4) && cols[1] && !cols[2] && cols[3]) {
                root = insert_station(root, cols[1], atol(cols[3]), 0, 0);
            }

            // Cas 2 : Connexion Source -> Usine (Volumes)
            // Concerne les modes : "src" (2), "real" (3) et "all" (4)
            if ((mode_histo == 2 || mode_histo == 3 || mode_histo == 4) && cols[2] && cols[3]) {
                long vol = atol(cols[3]);
                long reel = vol;
                // Calcul des fuites si nécessaire (pour real ou all)
                if ((mode_histo == 3 || mode_histo == 4) && cols[4]) {
                    double p_leak = atof(cols[4]);
                    reel = (long)(vol * (1.0 - (p_leak / 100.0)));
                }
                root = insert_station(root, cols[2], 0, vol, reel);
            }
        }
    }

    fclose(file);

    // Sorties
    if (mode_leaks) {
        Station* start = find_station(root, arg_mode);
        if (!start) {
            printf("-1\n");
        } else {
            double starting_volume = (start->real_qty > 0) ? (double)start->real_qty : (double)start->capacity;
            double leaks = solve_leaks(start, starting_volume);
            printf("%.6f\n", leaks / 1000.0);
        }
    } else {
        char mode_str[10];
        if (mode_histo == 1) strcpy(mode_str, "max");
        if (mode_histo == 2) strcpy(mode_str, "src");
        if (mode_histo == 3) strcpy(mode_str, "real");
        if (mode_histo == 4) strcpy(mode_str, "all"); // AJOUT
        write_csv(root, stdout, mode_str);
    }
    
    free_tree(root);
    if(big_buffer) free(big_buffer);
    return 0;
}
