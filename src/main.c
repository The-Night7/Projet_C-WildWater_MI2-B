/*
 * main.c - VERSION FINALE (Optimisée + Bonus All + Bonus Leak Max)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "avl.h"

// -----------------------------------------------------------------------------
//  Fonction récursive de calcul des fuites (DFS)
//  MODIF BONUS : On passe des pointeurs pour suivre le tronçon critique
// -----------------------------------------------------------------------------
static double solve_leaks(Station* node, double input_vol, 
                          double* max_leak_val, char** max_from, char** max_to) {
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

        // --- BONUS 2 : Détection du pire tronçon (Valeur Absolue) ---
        if (pipe_loss > *max_leak_val) {
            *max_leak_val = pipe_loss;
            *max_from = node->name;       // Identifiant Amont
            *max_to = curr->target->name; // Identifiant Aval
        }
        // ------------------------------------------------------------

        // Ce qui arrive réellement à l’enfant
        double vol_arrived = vol_per_pipe - pipe_loss;
        
        // Somme : perte locale + pertes des enfants (appel récursif)
        total_loss += pipe_loss + solve_leaks(curr->target, vol_arrived, max_leak_val, max_from, max_to);
        
        curr = curr->next;
    }
    return total_loss;
}

// -----------------------------------------------------------------------------
//  Main
// -----------------------------------------------------------------------------
int main(int argc, char** argv) {
    if (argc != 3) return 1;
    
    FILE* file = fopen(argv[1], "r");
    if (!file) return 2;

    // --- OPTIMISATION I/O ---
    const size_t BUF_SIZE = 16 * 1024 * 1024; // 16 Mo
    char* big_buffer = malloc(BUF_SIZE);
    if (big_buffer) setvbuf(file, big_buffer, _IOFBF, BUF_SIZE);

    char* arg_mode = argv[2];
    int mode_histo = 0; // 1=max, 2=src, 3=real, 4=all
    int mode_leaks = 0;
    
    if (strcmp(arg_mode, "max") == 0) mode_histo = 1;
    else if (strcmp(arg_mode, "src") == 0) mode_histo = 2;
    else if (strcmp(arg_mode, "real") == 0) mode_histo = 3;
    else if (strcmp(arg_mode, "all") == 0) mode_histo = 4;
    else mode_leaks = 1;

    Station* root = NULL;
    char line[1024];
    long line_count = 0;

    while (fgets(line, sizeof(line), file)) {
        line_count++;
        if (line_count % 200000 == 0) {
            fprintf(stderr, "Lignes traitees : %ld...\r", line_count);
            fflush(stderr);
        }

        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) < 2) continue;

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

        // --- TRIM DES ESPACES ---
        for (int i = 0; i < 5; i++) {
            if (cols[i]) {
                while (*cols[i] == ' ') cols[i]++;
                char* end = cols[i] + strlen(cols[i]) - 1;
                while (end > cols[i] && *end == ' ') {
                    *end = '\0';
                    end--;
                }
                if (strcmp(cols[i], "-") == 0 || strlen(cols[i]) == 0) {
                    cols[i] = NULL;
                }
            }
        }

        if (mode_leaks) {
            // Logique optimisée (1 recherche par noeud)
            Station* pa = NULL;
            Station* ch = NULL;

            if (cols[1]) {
                pa = find_station(root, cols[1]);
                if (!pa) {
                    root = insert_station(root, cols[1], 0, 0, 0);
                    pa = find_station(root, cols[1]);
                }
            }
            if (cols[2]) {
                ch = find_station(root, cols[2]);
                if (!ch) {
                    root = insert_station(root, cols[2], 0, 0, 0);
                    ch = find_station(root, cols[2]);
                }
            }
            if (pa && ch) {
                double leak = (cols[4]) ? atof(cols[4]) : 0.0;
                add_connection(pa, ch, leak);
                // Volume réel entrant pour l'usine cible
                if (cols[3] && strcmp(ch->name, arg_mode) == 0) {
                    double vol = atof(cols[3]);
                    ch->real_qty += (long)(vol * (1.0 - leak/100.0));
                }
            }
            // Mise à jour capacité
            if (pa && !cols[2] && cols[3]) {
                pa->capacity = atol(cols[3]);
            }

        } else {
            // Logique Histogrammes (max, src, real, all)
            // Définition Usine (max, all)
            if ((mode_histo == 1 || mode_histo == 4) && cols[1] && !cols[2] && cols[3]) {
                root = insert_station(root, cols[1], atol(cols[3]), 0, 0);
            }
            // Données Volumes (src, real, all)
            if ((mode_histo == 2 || mode_histo == 3 || mode_histo == 4) && cols[2] && cols[3]) {
                long vol = atol(cols[3]);
                long reel = vol;
                if ((mode_histo == 3 || mode_histo == 4) && cols[4]) {
                    double p_leak = atof(cols[4]);
                    reel = (long)(vol * (1.0 - (p_leak / 100.0)));
                }
                root = insert_station(root, cols[2], 0, vol, reel);
            }
        }
    }

    fclose(file);

    // Sorties Finales
    if (mode_leaks) {
        Station* start = find_station(root, arg_mode);
        if (!start) {
            printf("-1\n");
        } else {
            // Variables pour le Bonus
            double max_leak_val = 0.0;
            char* max_from = NULL;
            char* max_to = NULL;

            double starting_volume = (start->real_qty > 0) ? (double)start->real_qty : (double)start->capacity;
            
            // Appel avec les pointeurs bonus
            double leaks = solve_leaks(start, starting_volume, &max_leak_val, &max_from, &max_to);
            
            // Affichage Standard (pour le script bash)
            printf("%.6f\n", leaks / 1000.0);

            // --- AFFICHAGE BONUS (sur stderr pour ne pas casser le CSV) ---
            if (max_from && max_to) {
                fprintf(stderr, "\n=== INFO BONUS ===\n");
                fprintf(stderr, "Troncon critique (Pire fuite absolue) :\n");
                fprintf(stderr, "Amont : %s\n", max_from);
                fprintf(stderr, "Aval  : %s\n", max_to);
                fprintf(stderr, "Perte : %.6f M.m3\n", max_leak_val / 1000.0);
                fprintf(stderr, "==================\n");
            }
        }
    } else {
        // Mode Histo
        char mode_str[10];
        if (mode_histo == 1) strcpy(mode_str, "max");
        if (mode_histo == 2) strcpy(mode_str, "src");
        if (mode_histo == 3) strcpy(mode_str, "real");
        if (mode_histo == 4) strcpy(mode_str, "all");
        write_csv(root, stdout, mode_str);
    }
    
    free_tree(root);
    if(big_buffer) free(big_buffer);
    return 0;
}
