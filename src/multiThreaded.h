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
 */
#define maxthreads 4

/**
 * Global timing variables
 */
extern clock_t thread_start, thread_stop;

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
} Threads;

/**
 * Initialize a node group
 * @param ng Pointer to node group
 */
void initNodeGroup(NodeGroup* ng);

/**
 * Thread function to process tasks
 * @param arg Pointer to node group containing tasks
 * @return NULL
 */
void* doallTasks(void* arg);

/**
 * Add a task to a node group
 * @param g Node group
 * @param task Task to add
 */
void addTaskToGroup(NodeGroup g, Task* task);

/**
 * Add a task to the least loaded thread
 * @param t Thread system
 * @param task Function to execute
 * @param data Data to pass to function
 */
void addTaskInThreads(Threads* t, void (*task)(void* param), void* data);

/**
 * Execute all tasks in the thread system
 * @param t Thread system
 */
void handleThreads(Threads* t);

/**
 * Create and initialize a thread system
 * @return Pointer to initialized thread system
 */
Threads* setupThreads();

/**
 * Add content to a node group
 * @param ng Pointer to node group
 * @param content Content to add
 */
void addContent(NodeGroup* ng, void* content);

#endif /* MULTITHREADED_H */