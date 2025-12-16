#include "multiThreaded.h"

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
    if (!schedule->head) return NULL;
    
    Node* current = schedule->head;
    Node* temp = NULL;
    int tasks_executed = 0;
    
    while (current) {
        if (current->content) {
            // Execute the task with its data
            Task* tsk = (Task*)current->content;
            tsk->task(tsk->data);
            tasks_executed++;

            free(tsk);
        }
        temp = current;
        schedule->head = current->next;
        current = current->next;
        free(temp);
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
    ng->head = malloc(sizeof(Node));
    if (!ng->head) {
        return -1;
    }
    ng->head->content = NULL;
    ng->head->next = NULL;
    if (pthread_mutex_init(&ng->mutex, NULL) != 0) {
        free(ng->head);
        ng->head = NULL;
        return -1;
    }
    return 0;
}

/**
 * Create and initialize a thread system
 *
 * @return Pointer to the new Threads system, or NULL on failure
 */
Threads* setupThreads() {
    Threads* newThreads = malloc(sizeof(Threads));
    if (!newThreads) return NULL;
    newThreads->doall = doallTasks;
    newThreads->error_count = 0;
    for (int i = 0; i < maxthreads; i++) {
        newThreads->occupency[i] = 0;
        if (initNodeGroup(&newThreads->scheduledTasks[i]) != 0) {
            // Free previously initialized groups
            for (int j = 0; j < i; j++) {
                cleanupNodeGroup(&newThreads->scheduledTasks[j]);
            }
            free(newThreads);
            return NULL;
        }
    }
    return newThreads;
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
    pthread_mutex_lock(&g->mutex);
    Node* current = g->head;
    // Ensure there is a sentinel node
    if (!current) {
        g->head = malloc(sizeof(Node));
        if (!g->head) {
            pthread_mutex_unlock(&g->mutex);
            return -1;
        }
        g->head->content = NULL;
        g->head->next = NULL;
        current = g->head;
    }
    while (current->next) {
        current = current->next;
    }
    Node* new = malloc(sizeof(Node));
    if (!new) {
        pthread_mutex_unlock(&g->mutex);
        return -1;
    }
    new->next = NULL;
    new->content = (void*)task;
    current->next = new;
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
    int min = t->occupency[0];
    int slot = 0;
    for (int i = 1; i < maxthreads; i++) {
        if (t->occupency[i] < min) {
            min = t->occupency[i];
            slot = i;
        }
    }
    // Allocate and add the new task
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
        int rc = pthread_create(&t->threads[i], NULL, t->doall, (void*)&t->scheduledTasks[i]);
        if (rc != 0) {
            err++;
        }
    }
    for (int i = 0; i < maxthreads; i++) {
        int rc = pthread_join(t->threads[i], NULL);
        if (rc != 0) {
            err++;
        }
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
    pthread_mutex_lock(&ng->mutex);
    Node* current = ng->head;
    // Create sentinel if needed
    if (!current) {
        ng->head = malloc(sizeof(Node));
        if (!ng->head) {
            pthread_mutex_unlock(&ng->mutex);
            return -1;
        }
        ng->head->content = NULL;
        ng->head->next = NULL;
        current = ng->head;
    }
    while (current->next) {
        current = current->next;
    }
    Node* node = malloc(sizeof(Node));
    if (!node) {
        pthread_mutex_unlock(&ng->mutex);
        return -1;
    }
    node->content = content;
    node->next = NULL;
    current->next = node;
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
    while (current) {
        Node* tmp = current;
        current = current->next;
        free(tmp);
    }
    ng->head = NULL;
    pthread_mutex_unlock(&ng->mutex);
    pthread_mutex_destroy(&ng->mutex);
}

/**
 * Clean up a thread system and free its resources
 *
 * @param t Pointer to the thread system to clean up
 */
void cleanupThreads(Threads* t) {
    if (!t) return;
    for (int i = 0; i < maxthreads; i++) {
        cleanupNodeGroup(&t->scheduledTasks[i]);
    }
    // Note: global_mutex is not destroyed as it may be used by other thread systems
    free(t);
}