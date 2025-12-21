/*
 * main.c - Optimized water network analysis program
 *
 * Analysis program for water distribution networks that features:
 * - Histogram generation for capacities and water volumes
 * - Calculation of water losses downstream from specific facilities
 * - Detection of critical sections with highest leakage
 * - Optimized sequential processing for better performance
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <immintrin.h>  // Pour les instructions SIMD
#include "avl.h"
#include "structs.h"

// Variable globale pour mesurer le temps
clock_t process_start, process_stop;

/**
 * Removes whitespace at the beginning and end of string
 * @param str String to trim
 * @return Pointer to trimmed string
 */
static char* trim_whitespace(char* str) {
    if (!str) return NULL;
    // Move to first non-whitespace character
    while (*str && (*str == ' ' || *str == '\t')) {
        str++;
    }
    if (*str == '\0') return str;
    // Remove trailing whitespace
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t')) {
        *end = '\0';
        end--;
    }
    return str;
}

/**
 * Optimized recursive calculation of water losses in the network
 *
 * @param node         Current station
 * @param input_vol    Incoming water volume
 * @param u            Facility for which leaks are calculated
 * @param max_leak_val Pointer to track maximum leak value
 * @param max_from     Pointer to track upstream station of critical section
 * @param max_to       Pointer to track downstream station of critical section
 * @return             Total downstream leak volume
 */
static double solve_leaks_optimized(Station* node, double input_vol, Station* u,
                         double* max_leak_val, char** max_from, char** max_to) {
    // Conditions d'arrêt précoce avec seuil plus élevé pour éviter les calculs inutiles
    if (!node || input_vol <= 0.01) return 0.0;
    if (node->nb_children == 0) return 0.0;

    // Pré-calcul du nombre de connexions valides et pré-allocation des tableaux
    #define MAX_LOCAL_CONNECTIONS 32
    AdjNode* valid_connections[MAX_LOCAL_CONNECTIONS];
    double pipe_losses[MAX_LOCAL_CONNECTIONS];
    double volumes_arrived[MAX_LOCAL_CONNECTIONS];

    int valid_count = 0;
    AdjNode* curr = node->children;
    
    // Premier passage: compter et collecter les connexions valides
    while (curr && valid_count < MAX_LOCAL_CONNECTIONS) {
        if (curr->factory == NULL || curr->factory == u) {
            valid_connections[valid_count++] = curr;
        }
        curr = curr->next;
    }
    
    if (valid_count == 0) return 0.0;

    // Distribution du volume et calcul des pertes
    double total_loss = 0.0;
    double vol_per_pipe = input_vol / valid_count;

    // Pré-calcul des pertes et volumes pour chaque connexion
    for (int i = 0; i < valid_count; i++) {
        curr = valid_connections[i];

        // Pré-calcul des pertes
        if (curr->leak_perc > 0.01) {
            pipe_losses[i] = vol_per_pipe * (curr->leak_perc / 100.0);
            } else {
            pipe_losses[i] = 0.0;
            }
            
        // Suivi de la fuite maximale
        if (pipe_losses[i] > *max_leak_val) {
            *max_leak_val = pipe_losses[i];
            *max_from = node->name;
            *max_to = curr->target->name;
        }

        // Pré-calcul des volumes arrivés
        volumes_arrived[i] = vol_per_pipe - pipe_losses[i];

        // Accumulation des pertes
        total_loss += pipe_losses[i];
    }
    
    // Traitement récursif des branches avec volume significatif
    for (int i = 0; i < valid_count; i++) {
        if (volumes_arrived[i] > 0.01) {
            total_loss += solve_leaks_optimized(
                valid_connections[i]->target,
                volumes_arrived[i],
                u,
                max_leak_val,
                max_from,
                max_to
            );
        }
    }

    return total_loss;
}
/**
 * Program entry point
 *
 * Arguments:
 * - argv[1]: path to data file (.dat or .csv)
 * - argv[2]: execution mode
 *   * "max", "src", "real", "all": histogram generation
 *   * other: facility ID for specific leak calculation
 */
int main(int argc, char** argv) {
    // Argument validation
    if (argc != 3) return 1;

    // Open data file
    FILE* file = fopen(argv[1], "r");
    if (!file) return 2;

    // Reading optimization
    const size_t BUF_SIZE = 64 * 1024 * 1024; // 64 MB buffer
    char* big_buffer = malloc(BUF_SIZE);
    if (big_buffer) setvbuf(file, big_buffer, _IOFBF, BUF_SIZE);

    // Determine execution mode
    char* arg_mode = argv[2];
    int mode_histo = 0; // 1=max, 2=src, 3=real, 4=all
    int mode_leaks = 0;

    if (strcmp(arg_mode, "max") == 0) mode_histo = 1;
    else if (strcmp(arg_mode, "src") == 0) mode_histo = 2;
    else if (strcmp(arg_mode, "real") == 0) mode_histo = 3;
    else if (strcmp(arg_mode, "all") == 0) mode_histo = 4;
    else mode_leaks = 1; // Any other argument is considered a facility ID

    // Initialization
    Station* root = NULL;
    char line[1024];
    long line_count = 0;
    long station_count = 0;
    long capacity_count = 0;

    // Read and process file
    while (fgets(line, sizeof(line), file)) {
        line_count++;

        // Supprimer ou réduire les affichages de progression
                #ifdef DEBUG
        if (line_count % 5000000 == 0) {
            fprintf(stderr, "Lines processed: %ld...\r", line_count);
            fflush(stderr);
            }
        #endif

        // Nettoyage de ligne optimisé
        char* end = line + strcspn(line, "\r\n");
        *end = '\0';
        if (end == line) continue;  // Ligne vide

        // Parsing CSV optimisé
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

        // Clean fields - uniquement si nécessaire
        if (mode_leaks) {
            for (int i = 0; i < c; i++) {
                if (cols[i]) {
                    cols[i] = trim_whitespace(cols[i]);
                    if (cols[i][0] == '-' && cols[i][1] == '\0') {
                        cols[i] = NULL;
                    } else if (cols[i][0] == '\0') {
                        cols[i] = NULL;
                    }
                }
            }
        }

        // Process according to mode
    if (mode_leaks) {
            // Leak calculation mode: build complete graph

            // Create stations if needed
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

            // Create connections between stations
            if (pa && ch) {
                double leak = (cols[4] ? atof(cols[4]) : 0.0);  // Leak %

                // Determine facility associated with section
                Station* factory = NULL;
                if (cols[0]) {
                    // Explicitly mentioned facility
                    factory = find_station(root, cols[0]);
                    if (!factory) {
                        root = insert_station(root, cols[0], 0, 0, 0);
                        factory = find_station(root, cols[0]);
                        station_count++;
                    }
    } else {
                    // Implicit facility based on section type
                    if (cols[3]) {
                        factory = ch;  // Source→facility: facility is downstream
    } else {
                        factory = pa;  // Facility→storage: facility is upstream
                    }
                }

                // Add connection
                add_connection(pa, ch, leak, factory);

                // Update actual volume for source→facility sections
                if (cols[3] && !cols[0]) {
                    double vol = atof(cols[3]);
                    double real_vol = vol * (1.0 - leak / 100.0);

                    if (mode_leaks && ch && strcmp(ch->name, arg_mode) == 0) {
                        ch->real_qty += (long)real_vol;
                    }
                }
            }

            // Update facility capacities
            if (cols[1] && !cols[2] && cols[3]) {
                Station* s = find_station(root, cols[1]);
                if (s) {
                    s->capacity += atol(cols[3]);
                    capacity_count++;
                }
            }

        } else {
            // Histogram mode: aggregate according to mode

            if ((mode_histo == 1 || mode_histo == 4) && cols[1] && !cols[2] && cols[3]) {
                // "max" mode or "all" mode: maximum facility capacities
                root = insert_station(root, cols[1], atol(cols[3]), 0, 0);
            }

            if ((mode_histo == 2 || mode_histo == 4) && cols[2] && cols[3]) {
                // "src" mode or "all" mode: captured volumes
                long vol = atol(cols[3]);
                root = insert_station(root, cols[2], 0, vol, 0);
            }

            if ((mode_histo == 3 || mode_histo == 4) && cols[2] && cols[3]) {
                // "real" mode or "all" mode: actual volumes
                long vol = atol(cols[3]);
                long real = vol;

                if (cols[4]) {
                    // Apply leak %
                    double p_leak = atof(cols[4]);
                    real = (long)(vol * (1.0 - (p_leak / 100.0)));
                }

                root = insert_station(root, cols[2], 0, 0, real);
            }
        }
    }

    #ifdef DEBUG
    fprintf(stderr, "Lines processed: %ld\n", line_count);
    #endif
    fclose(file);

    // Produce results according to mode
    if (mode_leaks) {
        // Calculate leaks for a specific facility
        Station* start = find_station(root, arg_mode);

        if (!start) {
            // Facility not found
            printf("-1\n");
        } else {
            // Calculate leaks from actual volume or capacity if needed
            double starting_volume = (start->real_qty > 0) ? (double)start->real_qty : (double)start->capacity;
            double leaks = 0.0;

            if (starting_volume > 0) {
                #ifdef DEBUG
                fprintf(stderr, "Starting leak calculation for %s...\n", start->name);
                #endif

                // Mesure du temps
                process_start = clock();

                // Variables pour suivre la section critique
                double max_leak_val = 0.0;
                char* max_from = NULL;
                char* max_to = NULL;

                // Calcul des fuites avec l'algorithme optimisé
                leaks = solve_leaks_optimized(start, starting_volume, start,
                                             &max_leak_val, &max_from, &max_to);

                process_stop = clock();

                #ifdef DEBUG
                double time_spent = (double)(process_stop - process_start) / CLOCKS_PER_SEC;
                fprintf(stderr, "Calculation completed in %.2f seconds\n", time_spent);

                // Affichage de la section critique
                if (max_leak_val > 0.0) {
                    fprintf(stderr, "\n=== BONUS INFO ===\n");
                    fprintf(stderr, "Critical section (Worst absolute leak):\n");
                    fprintf(stderr, "Upstream: %s\n", max_from);
                    fprintf(stderr, "Downstream: %s\n", max_to);
                    fprintf(stderr, "Loss: %.6f M.m3\n", max_leak_val / 1000.0);
                    fprintf(stderr, "=================\n");
            }
                #endif
        }

            // Display result in millions of m³
            printf("%.6f\n", leaks / 1000.0);
        }
    } else {
        // Generate histogram
        char mode_str[10];
        if (mode_histo == 1) strcpy(mode_str, "max");
        else if (mode_histo == 2) strcpy(mode_str, "src");
        else if (mode_histo == 3) strcpy(mode_str, "real");
        else if (mode_histo == 4) strcpy(mode_str, "all");

        write_csv(root, stdout, mode_str);
    }

    // Free memory
    free_tree(root);
    if (big_buffer) free(big_buffer);

    return 0;
}