<<<<<<< HEAD
<<<<<<< HEAD
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

=======
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
        
>>>>>>> origin/teuteu_test
        curr = curr->next;
    }
    return total_loss;
}

<<<<<<< HEAD
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
=======
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
>>>>>>> origin/teuteu_test

    Station* root = NULL;
    char line[1024];
    long line_count = 0;

<<<<<<< HEAD
    // --- Lecture du fichier ---
    while (fgets(line, sizeof(line), file)) {
        line_count++;
        // Feedback visuel utile pour les gros fichiers
=======
    while (fgets(line, sizeof(line), file)) {
        line_count++;
>>>>>>> origin/teuteu_test
        if (line_count % 200000 == 0) {
            fprintf(stderr, "Lignes traitees : %ld...\r", line_count);
            fflush(stderr);
        }

<<<<<<< HEAD
        line[strcspn(line, "\r\n")] = 0; 
        if (strlen(line) < 2) continue;

        // Découpage manuel (plus robuste que strtok pour les champs vides ";;")
        char* cols[5] = {NULL};
        char* p = line;
        int c = 0;
        
=======
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) < 2) continue;

        char* cols[5] = {NULL};
        char* p = line;
        int c = 0;
>>>>>>> origin/teuteu_test
        cols[c++] = p;
        while (*p && c < 5) {
            if (*p == ';') {
                *p = '\0';
                cols[c++] = p + 1;
            }
            p++;
        }

<<<<<<< HEAD
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
=======
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
>>>>>>> origin/teuteu_test
                root = insert_station(root, cols[2], 0, vol, reel);
            }
        }
    }
<<<<<<< HEAD
    
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
=======
/*
 * main.c - VERSION FINALE (Optimisée + Bonus All + Bonus Leak Max)
 *
 * Programme d'analyse du réseau d'eau potable C-WildWater.
 * Fonctionnalités:
 * - Génération d'histogrammes (capacités, volumes captés, volumes réels)
 * - Calcul des pertes en aval d'une usine spécifique ou de toutes les usines
 * - Détection du tronçon critique (pire fuite absolue)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "avl.h"

// Supprime les espaces en début et fin de chaîne
static char* trim_whitespace(char* str) {
    if (!str) return NULL;
    // Avancer jusqu'au premier caractère non blanc
    while (*str && (*str == ' ' || *str == '\t')) {
        str++;
    }
    if (*str == '\0') return str;
    // Supprimer les blancs en fin de chaîne
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t')) {
        *end = '\0';
        end--;
    }
    return str;
}

// Définition pour le mode "all"
#define ALL_LEAKS

/**
 * Calcule récursivement les pertes d'eau dans le réseau
 * Version améliorée avec détection du tronçon critique
 *
 * @param node         Station courante
 * @param input_vol    Volume d'eau entrant
 * @param u            Usine dont on calcule les fuites
 * @param max_leak_val Pointeur pour suivre la valeur de la fuite maximale
 * @param max_from     Pointeur pour suivre la station amont du tronçon critique
 * @param max_to       Pointeur pour suivre la station aval du tronçon critique
 * @return             Volume total de fuites en aval
 */
static double solve_leaks(Station* node, double input_vol, Station* u,
                         double* max_leak_val, char** max_from, char** max_to) {
    if (!node) return 0.0;

    // Compter les connexions sortantes pour cette usine
    int count = 0;
    AdjNode* curr = node->children;
    while (curr) {
        if (curr->factory == NULL || curr->factory == u) {
            count++;
        }
        curr = curr->next;
    }
    if (count == 0) return 0.0;

    // Répartir le volume et calculer les pertes
    double total_loss = 0.0;
    double vol_per_pipe = input_vol / count;
    curr = node->children;

    while (curr) {
        if (curr->factory == NULL || curr->factory == u) {
            // Calcul des pertes sur ce tronçon
            double pipe_loss = 0.0;
            if (curr->leak_perc > 0) {
                pipe_loss = vol_per_pipe * (curr->leak_perc / 100.0);
            }
            
            // --- BONUS : Détection du pire tronçon (Valeur Absolue) ---
            if (pipe_loss > *max_leak_val) {
                *max_leak_val = pipe_loss;
                *max_from = node->name;       // Identifiant Amont
                *max_to = curr->target->name; // Identifiant Aval
            }
            
            double vol_arrived = vol_per_pipe - pipe_loss;

            // Ajouter les pertes locales et récursives
            total_loss += pipe_loss + solve_leaks(curr->target, vol_arrived, u, 
                                                max_leak_val, max_from, max_to);
        }
        curr = curr->next;
    }
    return total_loss;
}

/**
 * Calcule les fuites pour toutes les usines de l'arbre
 *
 * @param node    Nœud courant de l'arbre
 * @param output  Fichier de sortie pour les résultats
 */
static void calculate_all_leaks(Station* node, FILE* output) {
    if (!node) return;

    // Parcours infixe (gauche-racine-droite)
    calculate_all_leaks(node->left, output);

    // Calcul des fuites si la station a un volume entrant
    double starting_volume = (double)node->real_qty;
    if (starting_volume > 0) {
        // Variables pour le bonus du tronçon critique
        double max_leak_val = 0.0;
        char* max_from = NULL;
        char* max_to = NULL;
        
        double leaks = solve_leaks(node, starting_volume, node, 
                                  &max_leak_val, &max_from, &max_to);
        
        // Conversion en millions de m³
        fprintf(output, "%s;%.6f\n", node->name, leaks / 1000.0);
        
        // Affichage du tronçon critique (sur stderr pour ne pas perturber le CSV)
        if (max_from && max_to && max_leak_val > 0.0) {
            fprintf(stderr, "Usine %s - Tronçon critique: %s → %s (%.6f M.m³)\n", 
                    node->name, max_from, max_to, max_leak_val / 1000.0);
        }
    }

    calculate_all_leaks(node->right, output);
}

// Compte le nombre total de stations dans l'arbre
static int count_stations(Station* node) {
    if (!node) return 0;
    return 1 + count_stations(node->left) + count_stations(node->right);
}

/**
 * Point d'entrée du programme
 *
 * Arguments:
 * - argv[1]: chemin du fichier de données (.dat ou .csv)
 * - argv[2]: mode d'exécution
 *   * "max", "src", "real", "all": génération d'histogrammes
 *   * autre: identifiant d'usine pour calcul de fuites spécifique
 */
int main(int argc, char** argv) {
    // Vérification des arguments
    if (argc != 3) return 1;

    // Ouverture du fichier de données
    FILE* file = fopen(argv[1], "r");
    if (!file) return 2;

    // Optimisation de la lecture
    const size_t BUF_SIZE = 16 * 1024 * 1024; // 16 Mo
    char* big_buffer = malloc(BUF_SIZE);
    if (big_buffer) setvbuf(file, big_buffer, _IOFBF, BUF_SIZE);

    // Détermination du mode d'exécution
    char* arg_mode = argv[2];
    int mode_histo = 0; // 1=max, 2=src, 3=real, 4=all
    int mode_leaks = 0;
    int mode_all_leaks = 0;

    if (strcmp(arg_mode, "max") == 0) mode_histo = 1;
    else if (strcmp(arg_mode, "src") == 0) mode_histo = 2;
    else if (strcmp(arg_mode, "real") == 0) mode_histo = 3;
    else if (strcmp(arg_mode, "all") == 0) mode_all_leaks = 1;
    else mode_leaks = 1; // Identifiant d'usine

    // Initialisation
    Station* root = NULL;
    char line[1024];
    long line_count = 0;

    // Intervalle d'affichage de la progression
#ifndef PROGRESS_INTERVAL
#define PROGRESS_INTERVAL 200000L
#endif
    long station_count = 0;
    long capacity_count = 0;

    // Lecture et traitement du fichier
    while (fgets(line, sizeof(line), file)) {
        line_count++;

        // Affichage périodique de la progression
        if (line_count % PROGRESS_INTERVAL == 0) {
            fprintf(stderr, "Lignes traitees : %ld...\r", line_count);
            fflush(stderr);
        }

        // Nettoyage de la ligne
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) < 2) continue;

        // Découpage par points-virgules (jusqu'à 5 colonnes)
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

        // Nettoyage des champs
        for (int i = 0; i < 5; i++) {
            if (cols[i]) {
                cols[i] = trim_whitespace(cols[i]);
                if (strcmp(cols[i], "-") == 0 || strlen(cols[i]) == 0) {
                    cols[i] = NULL;
                }
            }
        }

        // Traitement selon le mode
        if (mode_leaks || mode_all_leaks) {
            // Mode calcul de fuites: construction du graphe complet

            // Création des stations si nécessaire
            Station* pa = NULL;
            Station* ch = NULL;
            
            if (cols[1]) {
                pa = find_station(root, cols[1]);
                if (!pa) {
                    root = insert_station(root, cols[1], 0, 0, 0);
                    pa = find_station(root, cols[1]);
                    station_count++;
                }
            }
            
            if (cols[2]) {
                ch = find_station(root, cols[2]);
                if (!ch) {
                    root = insert_station(root, cols[2], 0, 0, 0);
                    ch = find_station(root, cols[2]);
                    station_count++;
                }
            }

            // Création des connexions entre stations
            if (pa && ch) {
                double leak = (cols[4] ? atof(cols[4]) : 0.0);  // % de fuite

                // Détermination de l'usine associée au tronçon
                Station* factory = NULL;
                if (cols[0]) {
                    // Usine explicitement mentionnée
                    factory = find_station(root, cols[0]);
                    if (!factory) {
                        root = insert_station(root, cols[0], 0, 0, 0);
                        factory = find_station(root, cols[0]);
                        station_count++;
                    }
                } else {
                    // Usine implicite selon le type de tronçon
                    if (cols[3]) {
                        factory = ch;  // Source→usine: l'usine est en aval
                    } else {
                        factory = pa;  // Usine→stockage: l'usine est en amont
                    }
                }

                // Ajout de la connexion
                add_connection(pa, ch, leak, factory);

                // Mise à jour du volume réel pour les tronçons source→usine
                if (cols[3] && !cols[0]) {
                    double vol = atof(cols[3]);
                    double real_vol = vol * (1.0 - leak / 100.0);

                    if (mode_all_leaks && ch) {
                        ch->real_qty += (long)real_vol;
                    } else if (mode_leaks && ch && strcmp(ch->name, arg_mode) == 0) {
                        ch->real_qty += (long)real_vol;
                    }
                }
            }

            // Mise à jour de la capacité des usines
            if (cols[1] && !cols[2] && cols[3]) {
                Station* s = find_station(root, cols[1]);
                if (s) {
                    s->capacity += atol(cols[3]);
                    capacity_count++;
                }
            }

        } else {
            // Mode histogramme: agrégation selon le mode choisi

            if (mode_histo == 1 && cols[1] && !cols[2] && cols[3]) {
                // Mode "max": capacités maximales des usines
                root = insert_station(root, cols[1], atol(cols[3]), 0, 0);
            }
            else if ((mode_histo == 2 || mode_histo == 3) && cols[2] && cols[3]) {
                // Modes "src" ou "real": volumes captés ou réels
                long vol = atol(cols[3]);
                long reel = vol;

                if (mode_histo == 3 && cols[4]) {
                    // Pour "real", appliquer le % de fuite
                    double p_leak = atof(cols[4]);
                    reel = (long)(vol * (1.0 - (p_leak / 100.0)));
                }

                root = insert_station(root, cols[2], 0, vol, reel);
            }
        }
    }

    // Affichage des statistiques en mode "all"
    if (mode_all_leaks) {
        fprintf(stderr, "Lignes traitées: %ld\n", line_count);
        fprintf(stderr, "Stations créées: %ld\n", station_count);
        fprintf(stderr, "Capacités définies: %ld\n", capacity_count);
        fprintf(stderr, "Nombre total de stations dans l'AVL: %d\n", count_stations(root));
    }

    fclose(file);

    // Production des résultats selon le mode
    if (mode_leaks) {
        // Calcul des fuites pour une usine spécifique
        Station* start = find_station(root, arg_mode);

        if (!start) {
            // Usine introuvable
            printf("-1\n");
        } else {
            // Variables pour le bonus du tronçon critique
            double max_leak_val = 0.0;
            char* max_from = NULL;
            char* max_to = NULL;
            
            // Calcul des fuites à partir du volume réel ou de la capacité si nécessaire
            double starting_volume = (start->real_qty > 0) ? (double)start->real_qty : (double)start->capacity;
            double leaks = 0.0;

            if (starting_volume > 0) {
                leaks = solve_leaks(start, starting_volume, start, 
                                   &max_leak_val, &max_from, &max_to);
            }

            // Affichage du résultat en millions de m³
            printf("%.6f\n", leaks / 1000.0);
            
            // --- AFFICHAGE BONUS (sur stderr pour ne pas casser le CSV) ---
            if (max_from && max_to && max_leak_val > 0.0) {
                fprintf(stderr, "\n=== INFO BONUS ===\n");
                fprintf(stderr, "Troncon critique (Pire fuite absolue) :\n");
                fprintf(stderr, "Amont : %s\n", max_from);
                fprintf(stderr, "Aval  : %s\n", max_to);
                fprintf(stderr, "Perte : %.6f M.m3\n", max_leak_val / 1000.0);
                fprintf(stderr, "==================\n");
            }
        }
    } else if (mode_all_leaks) {
        // Calcul des fuites pour toutes les usines
        calculate_all_leaks(root, stdout);
    } else {
        // Génération d'histogramme
        char mode_str[10];
        if (mode_histo == 1) strcpy(mode_str, "max");
        if (mode_histo == 2) strcpy(mode_str, "src");
        if (mode_histo == 3) strcpy(mode_str, "real");

        write_csv(root, stdout, mode_str);
    }

    // Libération de la mémoire
    free_tree(root);
    if (big_buffer) free(big_buffer);

>>>>>>> origin/main
    return 0;
}
=======

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
>>>>>>> origin/teuteu_test
