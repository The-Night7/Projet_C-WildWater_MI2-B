/*
 * main.c
 *
 * Point d’entrée du programme C‑WildWater.  Ce fichier lit un fichier de
 * données représentant le réseau d’eau potable, agrège les informations
 * demandées pour produire des histogrammes ou calcule les pertes en aval
 * d’une usine.  L’algorithme de parcours utilise un arbre AVL pour
 * stocker les usines triées et un graphe orienté pour représenter les
 * relations entre elles.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

// -----------------------------------------------------------------------------
//  Fonction récursive de calcul des fuites (DFS)
//
// À partir d’un nœud `node` et d’un volume d’entrée `input_vol`, calcule
// récursivement la quantité totale d’eau perdue en aval.  On répartit le
// volume d’entrée équitablement entre tous les enfants, on applique le
// pourcentage de fuite du tronçon et on additionne les pertes locales et
// celles des sous‑arbres.
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
        // Somme : perte locale + pertes des enfants
        total_loss += pipe_loss + solve_leaks(curr->target, vol_arrived);
        curr = curr->next;
    }
    return total_loss;
}

// -----------------------------------------------------------------------------
//  Main
//
// Le programme attend deux ou trois arguments sur la ligne de commande :
//  * argv[1] : chemin vers le fichier de données (.dat ou .csv)
//  * argv[2] : mode d’histogramme (« max », « src » ou « real »), ou
//              identifiant d’une usine pour le calcul des fuites
//
// En mode histogramme, un fichier CSV est écrit sur la sortie standard.
// En mode fuites, un nombre (double) représentant la perte en millions de
// mètres cubes est affiché.
int main(int argc, char** argv) {
    if (argc != 3) {
        return 1; // Code de retour > 0 si les arguments sont incorrects
    }
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        return 2; // Impossible d’ouvrir le fichier d’entrée
    }
    char* arg_mode = argv[2];
    int mode_histo = 0; // 1 = max, 2 = src, 3 = real
    int mode_leaks = 0;
    if (strcmp(arg_mode, "max") == 0) {
        mode_histo = 1;
    } else if (strcmp(arg_mode, "src") == 0) {
        mode_histo = 2;
    } else if (strcmp(arg_mode, "real") == 0) {
        mode_histo = 3;
    } else {
        mode_leaks = 1; // Tout autre argument est considéré comme un identifiant d’usine
    }
    Station* root = NULL;
    char line[1024];
    long line_count = 0;
    // Lecture ligne par ligne du fichier d’entrée
    while (fgets(line, sizeof(line), file)) {
        line_count++;
        // Affichage sur stderr toutes les 200 000 lignes pour suivre la progression
        if (line_count % 200000 == 0) {
            /*
             * Affichage périodique de la progression.  On imprime sur la
             * sortie d'erreur le nombre de lignes traitées toutes les
             * 200 000 lignes.  L'utilisation d'\r permet de réécrire la
             * même ligne dans le terminal.
             */
            fprintf(stderr, "Lignes traitees : %ld...\r", line_count);
            fflush(stderr);
        }
        // Nettoyage des retours chariot
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) < 2) {
            continue;
        }
        // Découpage manuel de la ligne par points‑virgules
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
        // Remplacer les tirets ou chaînes vides par NULL
        for (int i = 0; i < 5; i++) {
            if (cols[i] && (strcmp(cols[i], "-") == 0 || strlen(cols[i]) == 0)) {
                cols[i] = NULL;
            }
        }
        // -----------------------------------------------------------------
        // Mode « leaks » : construire le graphe et l’arbre des stations
        // -----------------------------------------------------------------
        if (mode_leaks) {
            // Associer les identifiants (colonne 2 et 3) à des stations
            if (cols[1]) {
                if (!find_station(root, cols[1])) {
                    root = insert_station(root, cols[1], 0, 0, 0);
                }
            }
            if (cols[2]) {
                if (!find_station(root, cols[2])) {
                    root = insert_station(root, cols[2], 0, 0, 0);
                }
            }
            // Création de la connexion parent→enfant
            if (cols[1] && cols[2]) {
                Station* pa = find_station(root, cols[1]);
                Station* ch = find_station(root, cols[2]);
                double leak = (cols[4]) ? atof(cols[4]) : 0.0;
                add_connection(pa, ch, leak);
            }
            // Mise à jour de la capacité de l’usine (colonne 4) si la colonne 3 est vide
            if (cols[1] && !cols[2] && cols[3]) {
                Station* s = find_station(root, cols[1]);
                if (s) {
                    s->capacity = atol(cols[3]);
                }
            }
        } else {
            // -----------------------------------------------------------------
            // Mode histogramme : agrégation selon le mode choisi
            // -----------------------------------------------------------------
            // Cas 1 : mode « max » : on récupère toutes les définitions d’usine
            // Structure: [Ignoré];[ID Usine];[Vide];[Capacité];[Ignoré]
            if (mode_histo == 1 && cols[1] && !cols[2] && cols[3]) {
                root = insert_station(root, cols[1], atol(cols[3]), 0, 0);
            }
            // Cas 2 : modes « src » et « real » : on agrège les volumes des sources
            // Structure: [Ignoré];[ID Source];[ID Usine];[Volume];[Fuite]
            else if ((mode_histo == 2 || mode_histo == 3) && cols[2] && cols[3]) {
                long vol = atol(cols[3]);
                long reel = vol;
                if (mode_histo == 3 && cols[4]) {
                    double p_leak = atof(cols[4]);
                    reel = (long)(vol * (1.0 - (p_leak / 100.0)));
                }
                root = insert_station(root, cols[2], 0, vol, reel);
            }
        }
    }
    /*
     * Ne pas afficher de message de fin de chargement sur stderr.  Le
     * fichier d'entrée peut compter plusieurs millions de lignes et un
     * message final n'est pas nécessaire pour l'utilisateur.  On
     * conserve néanmoins l'affichage périodique (toutes les 200 000
     * lignes) plus haut pour suivre la progression du traitement.
     */
    fclose(file);
    // Sorties finales
    if (mode_leaks) {
        Station* start = find_station(root, arg_mode);
        if (!start) {
            /*
             * Usine introuvable : renvoyer -1.  Cette convention est
             * utilisée par le script pour distinguer une absence de
             * résultat d'une usine ayant effectivement 0 M.m3 de pertes.
             */
            printf("-1\n");
        } else {
            double leaks = solve_leaks(start, (double)start->capacity);
            /*
             * Conversion : la capacité est exprimée en milliers de m³ dans
             * le fichier d'entrée.  On renvoie les pertes en millions de
             * m³ (division par 1000).
             */
            printf("%f\n", leaks / 1000.0);
        }
    } else {
        // Mode histogramme : écrire le CSV sur stdout
        char mode_str[10];
        if (mode_histo == 1) strcpy(mode_str, "max");
        if (mode_histo == 2) strcpy(mode_str, "src");
        if (mode_histo == 3) strcpy(mode_str, "real");
        write_csv(root, stdout, mode_str);
    }
    free_tree(root);
    return 0;
}
