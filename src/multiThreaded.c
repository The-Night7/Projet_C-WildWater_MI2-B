#include "multiThreaded.h"
#include <stdlib.h>

// Global timing variables
clock_t thread_start, thread_stop;

// Global mutex for protecting shared data
pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Thread function to process tasks
 * @param arg Pointer to node group containing tasks
 * @return NULL
 */
void* doallTasks(void* arg) {
    NodeGroup* schedule = (NodeGroup*)arg;
    if (!schedule) return NULL;

    // Détache toute la liste sous mutex (safe et rapide)
    pthread_mutex_lock(&schedule->mutex);
    Node* list = schedule->head;
    schedule->head = NULL;
    pthread_mutex_unlock(&schedule->mutex);

    // Traite hors mutex
    Node* current = list;
    while (current) {
        Node* next = current->next;
        Task* tsk = (Task*)current->content;
        if (tsk && tsk->task) {
            tsk->task(tsk->data);
        }
        free(tsk);
        free(current);

        current = next;
    }
    return NULL;
}

/**
 * Initialize a node group and its mutex
 *
 * @param ng Pointer to a NodeGroup to initialize
 * @return 0 on success, -1 on failure
 */
int initNodeGroup(NodeGroup* ng) {
    if (!ng) return -1;
    ng->head = NULL;
    if (pthread_mutex_init(&ng->mutex, NULL) != 0) return -1;
    return 0;
}
/**
 * Create and initialize a thread system
 *
 * @return Pointer to the new Threads system, or NULL on failure
 */
Threads* setupThreads() {
    Threads* t = malloc(sizeof(Threads));
    if (!t) return NULL;
    t->doall = doallTasks;
    t->error_count = 0;

    for (int i = 0; i < maxthreads; i++) {
        t->occupency[i] = 0;
        if (initNodeGroup(&t->scheduledTasks[i]) != 0) {
            for (int j = 0; j < i; j++) cleanupNodeGroup(&t->scheduledTasks[j]);
            free(t);
            return NULL;
        }
    }
    return t;
}

/**
 * Append a task to a node group
 *
 * @param g Pointer to the NodeGroup
 * @param task Task to add
 * @return 0 on success, -1 on failure
 */
int addTaskToGroup(NodeGroup* g, Task* task) {
    if (!g || !task) return -1;

    Node* n = malloc(sizeof(Node));
    if (!n) return -1;
    n->content = task;

    pthread_mutex_lock(&g->mutex);
    n->next = g->head;
    g->head = n;
    pthread_mutex_unlock(&g->mutex);

    return 0;
}

/**
 * Add a task to the least loaded thread
 *
 * @param t Pointer to the Threads system
 * @param task Function to execute in the worker
 * @param data Data to pass to the function
 * @return 0 on success, -1 on failure
 */
int addTaskInThreads(Threads* t, void (*task)(void* param), void* data) {
    if (!t || !task) return -1;

    pthread_mutex_lock(&global_mutex);

    int slot = 0;
    int min = t->occupency[0];
    for (int i = 1; i < maxthreads; i++) {
        if (t->occupency[i] < min) {
            min = t->occupency[i];
            slot = i;
        }
    }

    Task* ntsk = malloc(sizeof(Task));
    if (!ntsk) {
        pthread_mutex_unlock(&global_mutex);
        return -1;
    }
    ntsk->task = task;
    ntsk->data = data;

    if (addTaskToGroup(&t->scheduledTasks[slot], ntsk) != 0) {
        free(ntsk);
        pthread_mutex_unlock(&global_mutex);
        return -1;
    }

    t->occupency[slot]++;
    pthread_mutex_unlock(&global_mutex);
    return 0;
}

/**
 * Execute all queued tasks in the thread system
 *
 * @param t Thread system
 * @return 0 on success, >0 if one or more thread operations failed
 */
int handleThreads(Threads* t) {
    if (!t) return -1;
    int err = 0;

    for (int i = 0; i < maxthreads; i++) {
        if (pthread_create(&t->threads[i], NULL, t->doall, &t->scheduledTasks[i]) != 0)
            err++;
    }
    for (int i = 0; i < maxthreads; i++) {
        if (pthread_join(t->threads[i], NULL) != 0)
            err++;
    }

    t->error_count = err;
    return err;
}

/**
 * Append arbitrary content to a NodeGroup
 *
 * @param ng Pointer to NodeGroup
 * @param content Content to add
 * @return 0 on success, -1 on failure
 */
int addContent(NodeGroup* ng, void* content) {
    if (!ng) return -1;

    Node* n = malloc(sizeof(Node));
    if (!n) return -1;
    n->content = content;

    pthread_mutex_lock(&ng->mutex);
    n->next = ng->head;
    ng->head = n;
    pthread_mutex_unlock(&ng->mutex);

    return 0;
}

/**
 * Clean up a node group
 *
 * @param ng Pointer to the NodeGroup to clean up
 */
void cleanupNodeGroup(NodeGroup* ng) {
    if (!ng) return;

    pthread_mutex_lock(&ng->mutex);
    Node* current = ng->head;
    ng->head = NULL;
    pthread_mutex_unlock(&ng->mutex);

    while (current) {
        Node* next = current->next;
        free(current);
        current = next;
    }

    pthread_mutex_destroy(&ng->mutex);
}

/**
 * Clean up a thread system and free its resources
 *
 * @param t Pointer to the thread system to clean up
 */
void cleanupThreads(Threads* t) {
    if (!t) return;
    for (int i = 0; i < maxthreads; i++) cleanupNodeGroup(&t->scheduledTasks[i]);
    free(t);
}