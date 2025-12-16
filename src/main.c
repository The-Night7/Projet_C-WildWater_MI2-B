/*
 * main.c - Optimized version with multithreaded leak calculation
 *
 * Analysis program for the C-WildWater drinking water network.
 * Features:
 * - Histogram generation (capacities, captured volumes, actual volumes)
 * - Calculation of losses downstream from a specific facility
 * - Detection of critical section (worst absolute leak)
 * - Multithreaded processing for improved performance
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "avl.h"
#include "multiThreaded.h"

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
 * Optimized version with early termination for zero volume
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
    // Early termination for zero volume or null node
    if (!node || input_vol <= 0.001) return 0.0;

    // Early termination if no outgoing connections
    if (node->nb_children == 0) return 0.0;

    // Count valid outgoing connections for this facility
    int valid_count = 0;
    AdjNode* curr = node->children;
    
    while (curr) {
        if (curr->factory == NULL || curr->factory == u) {
            valid_count++;
        }
        curr = curr->next;
    }
    
    // Early termination if no valid connections
    if (valid_count == 0) return 0.0;

    // Distribute volume and calculate losses
    double total_loss = 0.0;
    double vol_per_pipe = input_vol / valid_count;
    curr = node->children;

    // Process each connection - optimized to minimize calculations
    while (curr) {
        // Only process/recurse if the pipe belongs to the requested facility (or is shared)
        if (curr->factory == NULL || curr->factory == u) {
            // Calculate losses on this section - only if leak percentage is significant
            double pipe_loss = 0.0;
            if (curr->leak_perc > 0.001) {
                pipe_loss = vol_per_pipe * (curr->leak_perc / 100.0);
            }

            // Track section with maximum leak - only update if significant
            if (pipe_loss > *max_leak_val) {
                *max_leak_val = pipe_loss;
                *max_from = node->name;       // Upstream ID
                *max_to = curr->target->name; // Downstream ID
            }

            double vol_arrived = vol_per_pipe - pipe_loss;
            
            // Early termination for negligible volume
            if (vol_arrived > 0.001) {
                // Add local and recursive losses
                total_loss += pipe_loss + solve_leaks(curr->target, vol_arrived, u,
                                                    max_leak_val, max_from, max_to);
            } else {
                // Just add the pipe loss without recursion
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
 * This improves performance by processing each outgoing branch in parallel
 * Optimized version with better memory management and early termination
 *
 * @param node     Starting station
 * @param volume   Input volume
 * @param facility Target facility
 * @return         Total leak volume
 */
static double calculate_leaks_mt(Station* node, double volume, Station* facility) {
    if (!node || volume <= 0.001) return 0.0;

    // Count valid outgoing connections
    int count = 0;
    AdjNode* curr = node->children;
    
    // Pre-allocate arrays for better performance
    AdjNode** valid_connections = NULL;
    double* pipe_losses = NULL;
    double* volumes_arrived = NULL;
    
    // First pass: count valid connections
    while (curr) {
        if (curr->factory == NULL || curr->factory == facility) {
            count++;
        }
        curr = curr->next;
    }
    
    if (count == 0) return 0.0;
    
    // If only one branch or few branches, use direct calculation for better performance
    if (count <= 2) {
        double max_leak_val = 0.0;
        char* max_from = NULL;
        char* max_to = NULL;
        return solve_leaks(node, volume, facility, &max_leak_val, &max_from, &max_to);
    }

    // Allocate arrays for connection data
    valid_connections = (AdjNode**)malloc(count * sizeof(AdjNode*));
    pipe_losses = (double*)malloc(count * sizeof(double));
    volumes_arrived = (double*)malloc(count * sizeof(double));
    
    if (!valid_connections || !pipe_losses || !volumes_arrived) {
        fprintf(stderr, "Memory allocation failed for connection arrays\n");
        free(valid_connections);
        free(pipe_losses);
        free(volumes_arrived);
        
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
            if (curr->leak_perc > 0.001) {
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
    
    // Create a NodeGroup to store results
    NodeGroup results;
    initNodeGroup(&results);
    
    // Prepare tasks for each branch - optimized to reduce memory allocations
    double total_pipe_loss = 0.0;
    double global_max_leak = 0.0;
    char* global_max_from = NULL;
    char* global_max_to = NULL;
    
    for (int i = 0; i < count; i++) {
        // Skip branches with negligible volume
        if (volumes_arrived[i] <= 0.001) {
            total_pipe_loss += pipe_losses[i];
            continue;
        }
        
        // Track maximum pipe loss
        if (pipe_losses[i] > global_max_leak) {
            global_max_leak = pipe_losses[i];
            global_max_from = node->name;
            global_max_to = valid_connections[i]->target->name;
        }
        
        total_pipe_loss += pipe_losses[i];
        
        // Create task for downstream calculation
        double* branch_result = malloc(sizeof(double));
        double* max_leak_val = malloc(sizeof(double));
        char** max_from = malloc(sizeof(char*));
        char** max_to = malloc(sizeof(char*));
        
        *branch_result = 0.0;  // Will be updated by the task
        *max_leak_val = 0.0;   // Initialize max leak
        *max_from = NULL;
        *max_to = NULL;
        
        // Create task data
        LeakTaskData* task_data = malloc(sizeof(LeakTaskData));
        task_data->node = valid_connections[i]->target;
        task_data->input_vol = volumes_arrived[i];
        task_data->facility = facility;
        task_data->leak_result = branch_result;
        task_data->max_leak_val = max_leak_val;
        task_data->max_from = max_from;
        task_data->max_to = max_to;
        
        // Add result to results list
        addContent(&results, task_data);
        
        // Schedule task
        addTaskInThreads(thread_system, leak_branch_task_wrapper, task_data);
    }
    
    // Free pre-allocated arrays
    free(valid_connections);
    free(pipe_losses);
    free(volumes_arrived);
    
    // Execute all tasks in parallel
    thread_start = clock();
    handleThreads(thread_system);
    thread_stop = clock();
    
    // Sum up results
    double downstream_leaks = 0.0;
    
    Node* current = results.head->next;  // Skip head node (empty)
    while (current) {
        LeakTaskData* data = (LeakTaskData*)current->content;
        if (data) {
            // Add branch result to total
            downstream_leaks += *(data->leak_result);
            
            // Update global max leak if needed
            if (*(data->max_leak_val) > global_max_leak) {
                global_max_leak = *(data->max_leak_val);
                global_max_from = *(data->max_from);
                global_max_to = *(data->max_to);
            }
            
            // Free allocated memory
            free(data->leak_result);
            free(data->max_leak_val);
            free(data->max_from);
            free(data->max_to);
            free(data);
        }
        
        Node* tmp = current;
        current = current->next;
        free(tmp);
    }
    
    // Display critical section info
    if (global_max_leak > 0.0) {
        fprintf(stderr, "\n=== BONUS INFO ===\n");
        fprintf(stderr, "Critical section (Worst absolute leak):\n");
        fprintf(stderr, "Upstream: %s\n", global_max_from);
        fprintf(stderr, "Downstream: %s\n", global_max_to);
        fprintf(stderr, "Loss: %.6f M.m3\n", global_max_leak / 1000.0);
        fprintf(stderr, "=================\n");
    }
    
    // Free thread system
    free(thread_system);
    
    // Return total leaks (pipe losses + downstream leaks)
    return total_pipe_loss + downstream_leaks;
}

/**
 * Counts total number of stations in the tree
 * @param node Root node
 * @return Number of stations
 */
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
    const size_t BUF_SIZE = 32 * 1024 * 1024; // 32 MB buffer for better performance
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

    // Progress display interval - reduced for more frequent updates
#ifndef PROGRESS_INTERVAL
#define PROGRESS_INTERVAL 100000L
#endif
    long station_count = 0;
    long capacity_count = 0;
    long last_report_time = time(NULL);

    // Read and process file
    while (fgets(line, sizeof(line), file)) {
        line_count++;

        // Periodic progress display - time-based to avoid excessive output
        time_t current_time = time(NULL);
        if (line_count % PROGRESS_INTERVAL == 0 || current_time > last_report_time) {
            fprintf(stderr, "Lines processed: %ld...\r", line_count);
            fflush(stderr);
            last_report_time = current_time;
        }

        // Clean line
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) < 2) continue;

        // Split by semicolons (up to 5 columns) - optimized parsing
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

        // Clean fields - only when needed
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

    fprintf(stderr, "Lines processed: %ld\n", line_count);
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
                fprintf(stderr, "Starting multithreaded leak calculation for %s...\n", start->name);
                // Use multithreaded calculation for better performance
                leaks = calculate_leaks_mt(start, starting_volume, start);
                double time_spent = (double)(thread_stop - thread_start) / CLOCKS_PER_SEC;
                fprintf(stderr, "Calculation completed in %.2f seconds\n", time_spent);
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