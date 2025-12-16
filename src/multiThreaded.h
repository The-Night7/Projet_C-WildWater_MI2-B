#ifndef MULTITHREADED_H
#define MULTITHREADED_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

/**
 * Number of threads to use for parallel processing
 * Can be adjusted based on the number of CPU cores
 */
#define maxthreads 4

/**
 * Global timing variables
 */
extern clock_t thread_start, thread_stop;

/**
 * Global mutex for protecting shared data
 */
extern pthread_mutex_t global_mutex;

/**
 * Basic node structure for linked lists
 */
typedef struct node {
    void* content;
    struct node* next;
} Node;

/**
 * Group of nodes forming a linked list
 */
typedef struct {
    Node* head;
    pthread_mutex_t mutex; // Mutex to protect node group operations
} NodeGroup;

/**
 * Task structure for thread execution
 */
typedef struct {
    void* data;
    void (*task)(void* param);
} Task;

/**
 * Thread management structure
 */
typedef struct {
    int occupency[maxthreads];
    NodeGroup scheduledTasks[maxthreads];
    pthread_t threads[maxthreads];
    void* (*doall)(void* self);
    int error_count;      // Count of thread errors
} Threads;

/**
 * Initialize a node group with mutex
 * @param ng Pointer to node group
 * @return 0 on success, -1 on failure
 */
int initNodeGroup(NodeGroup* ng);

/**
 * Clean up a node group and free resources
 * @param ng Pointer to node group
 */
void cleanupNodeGroup(NodeGroup* ng);

/**
 * Thread function to process tasks
 * @param arg Pointer to node group containing tasks
 * @return NULL
 */
void* doallTasks(void* arg);

/**
 * Add a task to a node group with mutex protection
 * @param g Node group
 * @param task Task to add
 * @return 0 on success, -1 on failure
 */
int addTaskToGroup(NodeGroup* g, Task* task);

/**
 * Add a task to the least loaded thread
 * @param t Thread system
 * @param task Function to execute
 * @param data Data to pass to function
 * @return 0 on success, -1 on failure
 */
int addTaskInThreads(Threads* t, void (*task)(void* param), void* data);

/**
 * Execute all tasks in the thread system
 * @param t Thread system
 * @return 0 on success, number of failed threads otherwise
 */
int handleThreads(Threads* t);

/**
 * Create and initialize a thread system
 * @return Pointer to initialized thread system, NULL on failure
 */
Threads* setupThreads();

/**
 * Clean up thread system and free resources
 * @param t Thread system to clean up
 */
void cleanupThreads(Threads* t);

/**
 * Add content to a node group with mutex protection
 * @param ng Pointer to node group
 * @param content Content to add
 * @return 0 on success, -1 on failure
 */
int addContent(NodeGroup* ng, void* content);

#endif /* MULTITHREADED_H */