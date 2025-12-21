/*
 * main.c - Multithreaded water network analysis program
 *
 * Analysis program for water distribution networks that features:
 * - Histogram generation for capacities and water volumes
 * - Calculation of water losses downstream from specific facilities
 * - Detection of critical sections with highest leakage
 * - Multithreaded processing for performance optimization
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <immintrin.h>  // Pour les instructions SIMD
#include "avl.h"
#include "multiThreaded.h"
#include "structs.h"

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
 * Recursively calculates water losses in the network
 * 
 * @param node         Current station
 * @param input_vol    Incoming water volume
 * @param u            Facility for which leaks are calculated
 * @param max_leak_val Pointer to track maximum leak value
 * @param max_from     Pointer to track upstream station of critical section
 * @param max_to       Pointer to track downstream station of critical section
 * @return             Total downstream leak volume
 */
static double solve_leaks(Station* node, double input_vol, Station* u,
                         double* max_leak_val, char** max_from, char** max_to) {
    // Conditions d'arrêt précoce avec seuil plus élevé pour éviter des calculs inutiles
    if (!node || input_vol <= 0.01) return 0.0;
    if (node->nb_children == 0) return 0.0;

    // Pré-calcul du nombre de connexions valides
    int valid_count = 0;
    AdjNode* curr = node->children;
    
    while (curr) {
        if (curr->factory == NULL || curr->factory == u) {
            valid_count++;
        }
        curr = curr->next;
    }
    
    if (valid_count == 0) return 0.0;

    // Distribution du volume et calcul des pertes
    double total_loss = 0.0;
    double vol_per_pipe = input_vol / valid_count;
    curr = node->children;

    // Traitement de chaque connexion avec seuils optimisés
    while (curr) {
        if (curr->factory == NULL || curr->factory == u) {
            double pipe_loss = 0.0;
            if (curr->leak_perc > 0.01) {  // Seuil augmenté
                pipe_loss = vol_per_pipe * (curr->leak_perc / 100.0);
            }

            if (pipe_loss > *max_leak_val) {
                *max_leak_val = pipe_loss;
                *max_from = node->name;
                *max_to = curr->target->name;
            }

            double vol_arrived = vol_per_pipe - pipe_loss;
            
            if (vol_arrived > 0.01) {  // Seuil augmenté
                total_loss += pipe_loss + solve_leaks(curr->target, vol_arrived, u,
                                                    max_leak_val, max_from, max_to);
            } else {
                total_loss += pipe_loss;
            }
        }
        curr = curr->next;
    }
    return total_loss;
}

/**
 * Thread task wrapper for leak calculation of a specific branch
 * @param arg Pointer to LeakTaskData structure
 */
static void leak_branch_task_wrapper(void* arg) {
    LeakTaskData* data = (LeakTaskData*)arg;

    // Execute leak calculation for this branch
    *(data->leak_result) = solve_leaks(
        data->node,
        data->input_vol,
        data->facility,
        data->max_leak_val,
        data->max_from,
        data->max_to
    );
}

/**
 * Calculates leaks for a facility using multithreading for branches
 * 
 * @param node     Starting station
 * @param volume   Input volume
 * @param facility Target facility
 * @return         Total leak volume
 */
static double calculate_leaks_mt(Station* node, double volume, Station* facility) {
    if (!node || volume <= 0.01) return 0.0;  // Seuil augmenté pour éviter les calculs inutiles
    // Count valid outgoing connections
    int count = 0;
    AdjNode* curr = node->children;
    
    // Préallouer des tableaux de taille fixe pour éviter les allocations dynamiques
    #define MAX_CONNECTIONS 128
    AdjNode* valid_connections[MAX_CONNECTIONS];
    double pipe_losses[MAX_CONNECTIONS];
    double volumes_arrived[MAX_CONNECTIONS];

    // First pass: count valid connections
    while (curr) {
        if (curr->factory == NULL || curr->factory == facility) {
            count++;
        }
        curr = curr->next;
    }
    
    if (count == 0) return 0.0;
    
    // Use direct calculation for small number of branches
    if (count <= 2) {
        double max_leak_val = 0.0;
        char* max_from = NULL;
        char* max_to = NULL;
        return solve_leaks(node, volume, facility, &max_leak_val, &max_from, &max_to);
    }

    // Check if we have enough space in our pre-allocated arrays
    if (count > MAX_CONNECTIONS) {
        // Fallback to direct calculation
        double max_leak_val = 0.0;
        char* max_from = NULL;
        char* max_to = NULL;
        return solve_leaks(node, volume, facility, &max_leak_val, &max_from, &max_to);
    }
    
    // Second pass: collect valid connections and pre-calculate losses
    curr = node->children;
    int idx = 0;
    double vol_per_pipe = volume / count;
    
    while (curr && idx < count) {
        if (curr->factory == NULL || curr->factory == facility) {
            valid_connections[idx] = curr;
            
            // Pre-calculate pipe loss
            if (curr->leak_perc > 0.01) {  // Seuil augmenté
                pipe_losses[idx] = vol_per_pipe * (curr->leak_perc / 100.0);
            } else {
                pipe_losses[idx] = 0.0;
            }
            
            // Pre-calculate volume that arrives
            volumes_arrived[idx] = vol_per_pipe - pipe_losses[idx];
            idx++;
        }
        curr = curr->next;
    }
    
    // Setup thread system for parallel processing
    Threads* thread_system = setupThreads();
    if (!thread_system) {
        double max_leak_val = 0.0;
        char* max_from = NULL;
        char* max_to = NULL;
        return solve_leaks(node, volume, facility, &max_leak_val, &max_from, &max_to);
    }

    // Create a NodeGroup to store results
    NodeGroup results;
    if (initNodeGroup(&results) != 0) {
        cleanupThreads(thread_system);
        double max_leak_val = 0.0;
        char* max_from = NULL;
        char* max_to = NULL;
        return solve_leaks(node, volume, facility, &max_leak_val, &max_from, &max_to);
    }

    // Prepare tasks for each branch
    double total_pipe_loss = 0.0;
        #ifdef DEBUG
    double global_max_leak = 0.0;
    char* global_max_from = NULL;
    char* global_max_to = NULL;
        #endif

    // Préallouer la mémoire pour les résultats des branches
    double branch_results[MAX_CONNECTIONS] = {0.0};
    double max_leak_vals[MAX_CONNECTIONS] = {0.0};
    char* max_froms[MAX_CONNECTIONS] = {NULL};
    char* max_tos[MAX_CONNECTIONS] = {NULL};
    LeakTaskData task_data_array[MAX_CONNECTIONS];
    for (int i = 0; i < count; i++) {
        // Skip branches with negligible volume
        if (volumes_arrived[i] <= 0.01) {  // Seuil augmenté
            total_pipe_loss += pipe_losses[i];
            continue;
        }
    #ifdef DEBUG
        // Track maximum pipe loss
        if (pipe_losses[i] > global_max_leak) {
            global_max_leak = pipe_losses[i];
            global_max_from = node->name;
            global_max_to = valid_connections[i]->target->name;
        }
    #endif

        total_pipe_loss += pipe_losses[i];

        // Utiliser la mémoire préallouée au lieu d'allouer dynamiquement
        task_data_array[i].node = valid_connections[i]->target;
        task_data_array[i].input_vol = volumes_arrived[i];
        task_data_array[i].facility = facility;
        task_data_array[i].leak_result = &branch_results[i];
        task_data_array[i].max_leak_val = &max_leak_vals[i];
        task_data_array[i].max_from = &max_froms[i];
        task_data_array[i].max_to = &max_tos[i];
        // Add result to results list
        addContent(&results, &task_data_array[i]);

        // Schedule task
        addTaskInThreads(thread_system, leak_branch_task_wrapper, &task_data_array[i]);
}

    // Execute all tasks in parallel
    thread_start = clock();
    int th_err = handleThreads(thread_system);
    if (th_err != 0) {
        // Erreur silencieuse en mode production
        #ifdef DEBUG
        fprintf(stderr, "Warning: %d thread operations failed\n", th_err);
        #endif
    }
    thread_stop = clock();

    // Sum up results
    double downstream_leaks = 0.0;

    // Traitement des résultats
    for (int i = 0; i < count; i++) {
        downstream_leaks += branch_results[i];
    #ifdef DEBUG
        if (max_leak_vals[i] > global_max_leak) {
            global_max_leak = max_leak_vals[i];
            global_max_from = max_froms[i];
            global_max_to = max_tos[i];
        }
    #endif
    }

    // NodeGroup cleanup is handled by cleanupNodeGroup
    cleanupNodeGroup(&results);
    cleanupThreads(thread_system);

    // Display critical section info only in debug mode
                #ifdef DEBUG
    if (global_max_leak > 0.0) {
        fprintf(stderr, "\n=== BONUS INFO ===\n");
        fprintf(stderr, "Critical section (Worst absolute leak):\n");
        fprintf(stderr, "Upstream: %s\n", global_max_from);
        fprintf(stderr, "Downstream: %s\n", global_max_to);
        fprintf(stderr, "Loss: %.6f M.m3\n", global_max_leak / 1000.0);
        fprintf(stderr, "=================\n");
    }
                #endif

    return total_pipe_loss + downstream_leaks;
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
                fprintf(stderr, "Starting multithreaded leak calculation for %s...\n", start->name);
                #endif

                // Use multithreaded calculation for better performance
                leaks = calculate_leaks_mt(start, starting_volume, start);

                #ifdef DEBUG
                double time_spent = (double)(thread_stop - thread_start) / CLOCKS_PER_SEC;
                fprintf(stderr, "Calculation completed in %.2f seconds\n", time_spent);
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