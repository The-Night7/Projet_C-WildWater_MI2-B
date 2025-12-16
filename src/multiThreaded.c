#include "multiThreaded.h"
#include <string.h>
#include <errno.h>

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
    if (!schedule || !schedule->head) return NULL;

    Node* current = NULL;
    Node* temp = NULL;
    int tasks_executed = 0;

    // Lock the node group to safely access its content
    pthread_mutex_lock(&schedule->mutex);

    current = schedule->head;

    while (current) {
        if (current->content) {
            Task* task = (Task*)current->content;

            // Unlock during task execution to allow other threads to work
            pthread_mutex_unlock(&schedule->mutex);

            // Execute the task
            task->task(task->data);
            tasks_executed++;

            // Free task memory
            free(task);

            // Lock again to continue list traversal
            pthread_mutex_lock(&schedule->mutex);
        }

        temp = current;
        current = current->next;
        free(temp);
    }

    // Reset the list head
    schedule->head = NULL;

    pthread_mutex_unlock(&schedule->mutex);
    return NULL;
}

/**
 * Initialize a node group with mutex
 * @param ng Pointer to node group
 * @return 0 on success, -1 on failure
 */
int initNodeGroup(NodeGroup* ng) {
    if (!ng) return -1;

    // Initialize mutex
    if (pthread_mutex_init(&ng->mutex, NULL) != 0) {
        fprintf(stderr, "Error initializing mutex: %s\n", strerror(errno));
        return -1;
    }

    // Create head node
    ng->head = malloc(sizeof(Node));
    if (!ng->head) {
        pthread_mutex_destroy(&ng->mutex);
        return -1;
    }

    ng->head->content = NULL;
    ng->head->next = NULL;
    return 0;
}

/**
 * Clean up a node group and free resources
 * @param ng Pointer to node group
 */
void cleanupNodeGroup(NodeGroup* ng) {
    if (!ng) return;

    pthread_mutex_lock(&ng->mutex);

    // Free all nodes
    Node* current = ng->head;
    while (current) {
        Node* temp = current;
        current = current->next;

        // Free task if present
        if (temp->content) {
            Task* task = (Task*)temp->content;
            free(task);
        }

        free(temp);
    }

    ng->head = NULL;

    pthread_mutex_unlock(&ng->mutex);
    pthread_mutex_destroy(&ng->mutex);
}

/**
 * Create and initialize a thread system
 * @return Pointer to initialized thread system, NULL on failure
 */
Threads* setupThreads() {
    Threads* newThreads = malloc(sizeof(Threads));
    if (!newThreads) {
        fprintf(stderr, "Failed to allocate memory for thread system\n");
        return NULL;
    }

    // Initialize thread system mutex
    if (pthread_mutex_init(&newThreads->mutex, NULL) != 0) {
        fprintf(stderr, "Error initializing thread system mutex: %s\n", strerror(errno));
        free(newThreads);
        return NULL;
    }

    newThreads->doall = doallTasks;
    newThreads->error_count = 0;

    // Initialize occupency array and task lists
    memset(newThreads->occupency, 0, sizeof(int) * maxthreads);

    int success = 1;
    for (int i = 0; i < maxthreads; i++) {
        if (initNodeGroup(&newThreads->scheduledTasks[i]) != 0) {
            success = 0;
            break;
        }
    }

    if (!success) {
        // Clean up on failure
        for (int i = 0; i < maxthreads; i++) {
            cleanupNodeGroup(&newThreads->scheduledTasks[i]);
        }
        pthread_mutex_destroy(&newThreads->mutex);
        free(newThreads);
        return NULL;
    }

    return newThreads;
}

/**
 * Clean up thread system and free resources
 * @param t Thread system to clean up
 */
void cleanupThreads(Threads* t) {
    if (!t) return;

    // Clean up each node group
    for (int i = 0; i < maxthreads; i++) {
        cleanupNodeGroup(&t->scheduledTasks[i]);
    }

    // Destroy mutex
    pthread_mutex_destroy(&t->mutex);

    // Free thread system
    free(t);
}

/**
 * Add a task to a node group with mutex protection
 * @param g Node group
 * @param task Task to add
 * @return 0 on success, -1 on failure
 */
int addTaskToGroup(NodeGroup* g, Task* task) {
    if (!g || !task) return -1;

    pthread_mutex_lock(&g->mutex);

    Node* current = g->head;
    while (current->next) {
        current = current->next;
    }

    Node* new_node = malloc(sizeof(Node));
    if (!new_node) {
        pthread_mutex_unlock(&g->mutex);
        return -1;
    }

    new_node->next = NULL;
    new_node->content = (void*)task;
    current->next = new_node;

    pthread_mutex_unlock(&g->mutex);
    return 0;
}

/**
 * Add a task to the least loaded thread
 * @param t Thread system
 * @param task Function to execute
 * @param data Data to pass to function
 * @return 0 on success, -1 on failure
 */
int addTaskInThreads(Threads* t, void (*task)(void* param), void* data) {
    if (!t || !task) return -1;

    pthread_mutex_lock(&t->mutex);

    // Find the least loaded thread
    int min = t->occupency[0];
    int slot = 0;
    for (int i = 0; i < maxthreads; i++) {
        if (t->occupency[i] < min) {
            min = t->occupency[i];
            slot = i;
        }
    }

    // Create and add the task
    Task* ntsk = malloc(sizeof(Task));
    if (!ntsk) {
        pthread_mutex_unlock(&t->mutex);
        return -1;
    }

    ntsk->task = task;
    ntsk->data = data;

    int result = addTaskToGroup(&t->scheduledTasks[slot], ntsk);
    if (result == 0) {
        t->occupency[slot]++;
    } else {
        free(ntsk);
    }

    pthread_mutex_unlock(&t->mutex);
    return result;
}

/**
 * Execute all tasks in the thread system
 * @param t Thread system
 * @return 0 on success, number of failed threads otherwise
 */
int handleThreads(Threads* t) {
    if (!t) return -1;

    int error_count = 0;

    // Create threads
    for (int i = 0; i < maxthreads; i++) {
        int result = pthread_create(&t->threads[i], NULL, t->doall, (void*)&t->scheduledTasks[i]);
        if (result != 0) {
            fprintf(stderr, "Error creating thread %d: %s\n", i, strerror(result));
            error_count++;
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < maxthreads; i++) {
        if (t->threads[i] != 0) {
            int result = pthread_join(t->threads[i], NULL);
            if (result != 0) {
                fprintf(stderr, "Error joining thread %d: %s\n", i, strerror(result));
                error_count++;
            }
        }
    }

    // Reset occupency
    pthread_mutex_lock(&t->mutex);
    memset(t->occupency, 0, sizeof(int) * maxthreads);
    pthread_mutex_unlock(&t->mutex);

    return error_count;
}

/**
 * Add content to a node group with mutex protection
 * @param ng Pointer to node group
 * @param content Content to add
 * @return 0 on success, -1 on failure
 */
int addContent(NodeGroup* ng, void* content) {
    if (!ng || !content) return -1;

    pthread_mutex_lock(&ng->mutex);

    Node* current = ng->head;
    while (current->next) {
        current = current->next;
    }

    Node* new_node = malloc(sizeof(Node));
    if (!new_node) {
        pthread_mutex_unlock(&ng->mutex);
        return -1;
    }

    new_node->content = content;
    new_node->next = NULL;
    current->next = new_node;

    pthread_mutex_unlock(&ng->mutex);
    return 0;
}