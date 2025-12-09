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
    if (argc != 3) return 1; 

    FILE* file = fopen(argv[1], "r");
    if (!file) return 2;

    char* arg_mode = argv[2];
    int mode_histo = 0; 
    int mode_leaks = 0; 

    // Détection du mode
    if (strcmp(arg_mode, "max") == 0) mode_histo = 1;
    else if (strcmp(arg_mode, "src") == 0) mode_histo = 2;
    else if (strcmp(arg_mode, "real") == 0) mode_histo = 3;
    else mode_leaks = 1; // Si ce n'est pas un mot clé, c'est un ID d'usine

    Station* root = NULL;
    char line[1024];
    long line_count = 0;

    // --- Lecture du fichier ---
    while (fgets(line, sizeof(line), file)) {
        line_count++;
        // Feedback visuel utile pour les gros fichiers
        if (line_count % 200000 == 0) {
            fprintf(stderr, "Lignes traitees : %ld...\r", line_count);
            fflush(stderr);
        }

        line[strcspn(line, "\r\n")] = 0; 
        if (strlen(line) < 2) continue;

        // Découpage manuel (plus robuste que strtok pour les champs vides ";;")
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

        // Remplacement des "-" par NULL pour simplifier les tests suivants
        for(int i=0; i<5; i++) {
            if (cols[i] && (strcmp(cols[i], "-") == 0 || strlen(cols[i]) == 0)) {
                cols[i] = NULL;
            }
        }

        // --- Logique de construction de l'arbre ---
        if (mode_leaks) {
            // 1. S'assurer que les stations existent
            if (cols[1] && !find_station(root, cols[1])) 
                root = insert_station(root, cols[1], 0, 0, 0);
            if (cols[2] && !find_station(root, cols[2])) 
                root = insert_station(root, cols[2], 0, 0, 0);

            // 2. Ajouter la connexion (Enfant et Fuite)
            if (cols[1] && cols[2]) {
                Station* pa = find_station(root, cols[1]);
                Station* ch = find_station(root, cols[2]);
                double leak = (cols[4]) ? atof(cols[4]) : 0.0;
                add_connection(pa, ch, leak);
            }

            // 3. Récupérer la capacité (si c'est la ligne de définition de l'usine)
            if (cols[1] && !cols[2] && cols[3]) {
                Station* s = find_station(root, cols[1]);
                if (s) s->capacity = atol(cols[3]);
            }
        }
        else { 
            // Mode Histo (Optimisé : on ne stocke que ce qui est nécessaire)
            
            // Cas A : Définition d'une usine (pour récupérer la capacité MAX)
            if (mode_histo == 1 && cols[1] && !cols[2] && cols[3]) {
                root = insert_station(root, cols[1], atol(cols[3]), 0, 0);
            }
            
            // Cas B : Lien Source -> Usine (pour SRC et REAL)
            // On vérifie que l'usine (col 2) existe dans la ligne
            else if ((mode_histo == 2 || mode_histo == 3) && cols[2] && cols[3]) {
                long vol = atol(cols[3]);
                long reel = vol;
                
                if (mode_histo == 3 && cols[4]) {
                    double p_leak = atof(cols[4]);
                    reel = (long)(vol * (1.0 - (p_leak/100.0)));
                }
                
                // On insère ou met à jour l'usine
                root = insert_station(root, cols[2], 0, vol, reel);
            }
        }
    }
    
    fclose(file); //  de fermer le fichier

    // --- Sortie des résultats  ---
    if (mode_leaks) {
        Station* start = find_station(root, arg_mode);
        if (!start) {
            // -1 pour indiquer "introuvable" 
            printf("-1\n"); 
        } else {
            // Calcul récursif des fuites
            double leaks = solve_leaks(start, (double)start->capacity);
            
            // Conversion : k.m3 (fichier) -> M.m3 (demandé) = division par 1000
            printf("%f\n", leaks / 1000.0); 
        }
    } else {
        // Mode Histo : Génération du CSV
        char mode_str[10];
        if (mode_histo == 1) strcpy(mode_str, "max");
        else if (mode_histo == 2) strcpy(mode_str, "src");
        else if (mode_histo == 3) strcpy(mode_str, "real");
        
        // Entête optionnelle si ton script ne l'ajoute pas déjà
        // fprintf(stdout, "Station;Volume\n"); 
        write_csv(root, stdout, mode_str);
    }

    free_tree(root);
    return 0;
}