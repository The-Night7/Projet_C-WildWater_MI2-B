/*
 * main.c - FINAL VERSION (Optimized + All Bonus + Max Leak Bonus + MultiThreaded)
 *
 * Analysis program for the C-WildWater drinking water network.
 * Features:
 * - Histogram generation (capacities, captured volumes, actual volumes)
 * - Calculation of losses downstream from a specific facility or all facilities
 * - Detection of critical section (worst absolute leak)
 * - Multithreaded processing for leak calculations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "avl.h"
#include "multiThreaded.h"

// Removes whitespace at the beginning and end of string
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

// Definition for "all" mode
#define ALL_LEAKS

/**
 * Recursively calculates water losses in the network
 * Enhanced version with critical section detection
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
    if (!node) return 0.0;

    // Count outgoing connections for this facility
    int count = 0;
    AdjNode* curr = node->children;
    while (curr) {
        if (curr->factory == NULL || curr->factory == u) {
            count++;
        }
        curr = curr->next;
    }
    if (count == 0) return 0.0;

    // Distribute volume and calculate losses
    double total_loss = 0.0;
    double vol_per_pipe = input_vol / count;
    curr = node->children;

    while (curr) {
        if (curr->factory == NULL || curr->factory == u) {
            // Calculate losses on this section
            double pipe_loss = 0.0;
            if (curr->leak_perc > 0) {
                pipe_loss = vol_per_pipe * (curr->leak_perc / 100.0);
            }

            // --- BONUS: Worst section detection (Absolute Value) ---
            if (pipe_loss > *max_leak_val) {
                *max_leak_val = pipe_loss;
                *max_from = node->name;       // Upstream ID
                *max_to = curr->target->name; // Downstream ID
            }

            double vol_arrived = vol_per_pipe - pipe_loss;

            // Add local and recursive losses
            total_loss += pipe_loss + solve_leaks(curr->target, vol_arrived, u,
                                                max_leak_val, max_from, max_to);
        }
        curr = curr->next;
    }
    return total_loss;
}

/**
 * Thread task wrapper for leak calculation
 * @param arg Pointer to LeakTaskData structure
 */
static void leak_task_wrapper(void* arg) {
    LeakTaskData* data = (LeakTaskData*)arg;

    // Execute leak calculation
    *(data->leak_result) = solve_leaks(
        data->node,
        data->input_vol,
        data->facility,
        data->max_leak_val,
        data->max_from,
        data->max_to
    );

    // Free the task data
    free(data);
}

/**
 * Calculates leaks for all facilities in the tree using multithreading
 *
 * @param node    Current tree node
 * @param output  Output file for results
 */
static void calculate_all_leaks(Station* node, FILE* output) {
    if (!node) return;

    // Setup thread system
    Threads* thread_system = setupThreads();

    // Create a NodeGroup to store results
    NodeGroup results;
    initNodeGroup(&results);

    // Recursive function to traverse tree and schedule tasks
    void schedule_leak_tasks(Station* current) {
        if (!current) return;

        // Process left subtree
        schedule_leak_tasks(current->left);

        // Calculate leaks if station has incoming volume
        double starting_volume = (double)current->real_qty;
        if (starting_volume > 0) {
            // Allocate memory for results and critical section tracking
            double* leak_result = malloc(sizeof(double));
            double* max_leak_val = malloc(sizeof(double));
            char** max_from = malloc(sizeof(char*));
            char** max_to = malloc(sizeof(char*));

            *leak_result = 0.0;
            *max_leak_val = 0.0;
            *max_from = NULL;
            *max_to = NULL;

            // Create task data
            LeakTaskData* task_data = malloc(sizeof(LeakTaskData));
            task_data->node = current;
            task_data->input_vol = starting_volume;
            task_data->facility = current;
            task_data->leak_result = leak_result;
            task_data->max_leak_val = max_leak_val;
            task_data->max_from = max_from;
            task_data->max_to = max_to;

            // Create result node to store station name and result pointers
            typedef struct {
                char* name;
                double* leak_result;
                double* max_leak_val;
                char** max_from;
                char** max_to;
            } ResultData;

            ResultData* result_data = malloc(sizeof(ResultData));
            result_data->name = current->name;
            result_data->leak_result = leak_result;
            result_data->max_leak_val = max_leak_val;
            result_data->max_from = max_from;
            result_data->max_to = max_to;

            // Add result to results list
            addContent(&results, result_data);

            // Schedule task
            addTaskInThreads(thread_system, leak_task_wrapper, task_data);
        }

        // Process right subtree
        schedule_leak_tasks(current->right);
    }

    // Schedule all tasks
    schedule_leak_tasks(node);

    // Execute all tasks in parallel
    handleThreads(thread_system);

    // Write results to output
    Node* current = results.head->next;  // Skip head node (empty)
    while (current) {
        typedef struct {
            char* name;
            double* leak_result;
            double* max_leak_val;
            char** max_from;
            char** max_to;
        } ResultData;

        ResultData* data = (ResultData*)current->content;
        if (data) {
            // Convert to millions of m³
            fprintf(output, "%s;%.6f\n", data->name, *(data->leak_result) / 1000.0);

            // Display critical section (optional)
            /*
            if (*(data->max_from) && *(data->max_to) && *(data->max_leak_val) > 0.0) {
                fprintf(stderr, "Facility %s - Critical section: %s → %s (%.6f M.m³)\n",
                        data->name, *(data->max_from), *(data->max_to),
                        *(data->max_leak_val) / 1000.0);
            }
            */

            // Free allocated memory
            free(data->leak_result);
            free(data->max_leak_val);
            free(data->max_from);
            free(data->max_to);
            free(data);
        }

        current = current->next;
    }

    // Free thread system
    free(thread_system);

    // Note: We don't need to call the original recursive functions since we've handled everything here
}

// Counts total number of stations in the tree
static int count_stations(Station* node) {
    if (!node) return 0;
    return 1 + count_stations(node->left) + count_stations(node->right);
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
    const size_t BUF_SIZE = 16 * 1024 * 1024; // 16 MB
    char* big_buffer = malloc(BUF_SIZE);
    if (big_buffer) setvbuf(file, big_buffer, _IOFBF, BUF_SIZE);

    // Determine execution mode
    char* arg_mode = argv[2];
    int mode_histo = 0; // 1=max, 2=src, 3=real, 4=all
    int mode_leaks = 0;
    int mode_all_leaks = 0;

    if (strcmp(arg_mode, "max") == 0) mode_histo = 1;
    else if (strcmp(arg_mode, "src") == 0) mode_histo = 2;
    else if (strcmp(arg_mode, "real") == 0) mode_histo = 3;
    else if (strcmp(arg_mode, "all") == 0) mode_histo = 4; // MODIFIÉ: "all" est maintenant un mode histogramme (4)
    else if (strcmp(arg_mode, "bonus") == 0) mode_leaks = 1;
    else mode_leaks = 1; // Facility ID

    // Initialization
    Station* root = NULL;
    char line[1024];
    long line_count = 0;

    // Progress display interval
#ifndef PROGRESS_INTERVAL
#define PROGRESS_INTERVAL 5L
#endif
    long station_count = 0;
    long capacity_count = 0;

    // Read and process file
    while (fgets(line, sizeof(line), file)) {
        line_count++;

        // Periodic progress display
        if (line_count % PROGRESS_INTERVAL == 0) {
            fprintf(stderr, "Lines processed: %ld...\r", line_count);
            fflush(stderr);
        }

        // Clean line
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) < 2) continue;

        // Split by semicolons (up to 5 columns)
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

        // Clean fields
        for (int i = 0; i < 5; i++) {
            if (cols[i]) {
                cols[i] = trim_whitespace(cols[i]);
                if (strcmp(cols[i], "-") == 0 || strlen(cols[i]) == 0) {
                    cols[i] = NULL;
                }
            }
        }

        // Process according to mode
        if (mode_leaks || mode_all_leaks) {
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

                    if (mode_all_leaks && ch) {
                        ch->real_qty += (long)real_vol;
                    } else if (mode_leaks && ch && strcmp(ch->name, arg_mode) == 0) {
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

                // Pour le mode "all", on ajoute seulement real_qty
                // Pour le mode "real", on ajoute à la fois consumption et real_qty
                if (mode_histo == 4) {
                    root = insert_station(root, cols[2], 0, 0, real);
                } else {
                    root = insert_station(root, cols[2], 0, 0, real);
                }
            }
        }
    }

    // Display statistics in "all" mode
    if (mode_all_leaks) {
        fprintf(stderr, "Lines processed: %ld\n", line_count);
        fprintf(stderr, "Stations created: %ld\n", station_count);
        fprintf(stderr, "Capacities defined: %ld\n", capacity_count);
        fprintf(stderr, "Total stations in AVL: %d\n", count_stations(root));
    }

    fclose(file);

    // Produce results according to mode
    if (mode_leaks) {
        // Calculate leaks for a specific facility
        Station* start = find_station(root, arg_mode);

        if (!start) {
            // Facility not found
            printf("-1\n");
        } else {
            // Variables for critical section bonus
            double max_leak_val = 0.0;
            char* max_from = NULL;
            char* max_to = NULL;

            // Calculate leaks from actual volume or capacity if needed
            double starting_volume = (start->real_qty > 0) ? (double)start->real_qty : (double)start->capacity;
            double leaks = 0.0;

            if (starting_volume > 0) {
                // For single facility, use the original non-threaded version
                leaks = solve_leaks(start, starting_volume, start,
                                   &max_leak_val, &max_from, &max_to);
            }

            // Display result in millions of m³
            printf("%.6f\n", leaks / 1000.0);

            // --- BONUS DISPLAY (to stderr to avoid breaking CSV) ---
            if (max_from && max_to && max_leak_val > 0.0) {
                fprintf(stderr, "\n=== BONUS INFO ===\n");
                fprintf(stderr, "Critical section (Worst absolute leak):\n");
                fprintf(stderr, "Upstream: %s\n", max_from);
                fprintf(stderr, "Downstream: %s\n", max_to);
                fprintf(stderr, "Loss: %.6f M.m3\n", max_leak_val / 1000.0);
                fprintf(stderr, "=================\n");
            }
        }
    } else if (mode_all_leaks) {
        // Calculate leaks for all facilities using multithreading
        fprintf(stderr, "Starting multithreaded leak calculation...\n");
        thread_start = clock(); // Start timing
        calculate_all_leaks(root, stdout);
        thread_stop = clock(); // End timing
        double time_spent = (double)(thread_stop - thread_start) / CLOCKS_PER_SEC;
        fprintf(stderr, "Multithreaded calculation completed in %.2f seconds\n", time_spent);
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