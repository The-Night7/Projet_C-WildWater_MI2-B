#include "multiThreaded.h"

// Global timing variables
clock_t thread_start, thread_stop;

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
            ((Task*)current->content)->task(((Task*)current->content)->data);
            tasks_executed++;
        }
        temp = current;
        schedule->head = current->next;
        current = current->next;
        free(temp);
    }
    
    return NULL;
}

/**
 * Initialize a node group
 * @param ng Pointer to node group
 */
void initNodeGroup(NodeGroup* ng) {
    ng->head = malloc(sizeof(Node));
    ng->head->content = NULL;
    ng->head->next = NULL;
}

/**
 * Create and initialize a thread system
 * @return Pointer to initialized thread system
 */
Threads* setupThreads() {
    Threads* newThreads = malloc(sizeof(Threads));
    newThreads->doall = doallTasks;
    
    for (int i = 0; i < maxthreads; i++) {
        newThreads->occupency[i] = 0;
        // Initialize the first node in each task list
        newThreads->scheduledTasks[i].head = malloc(sizeof(Node));
        newThreads->scheduledTasks[i].head->content = NULL;
        newThreads->scheduledTasks[i].head->next = NULL;
    }
    
    return newThreads;
}

/**
 * Add a task to a node group
 * @param g Node group
 * @param task Task to add
 */
void addTaskToGroup(NodeGroup g, Task* task) {
    Node* current = g.head;
    while (current->next) {
        current = current->next;
    }
    Node* new = malloc(sizeof(Node));
    new->next = NULL;
    new->content = (void*)task;
    current->next = new;
}

/**
 * Add a task to the least loaded thread
 * @param t Thread system
 * @param task Function to execute
 * @param data Data to pass to function
 */
void addTaskInThreads(Threads* t, void (*task)(void* param), void* data) {
    if (!t) return;
    
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
    ntsk->task = task;
    ntsk->data = data;
    addTaskToGroup(t->scheduledTasks[slot], ntsk);
    t->occupency[slot]++;
}

/**
 * Execute all tasks in the thread system
 * @param t Thread system
 */
void handleThreads(Threads* t) {
    if (!t) return;
    
    // Create threads
    for (int i = 0; i < maxthreads; i++) {
        pthread_create(&t->threads[i], NULL, t->doall, (void*)&t->scheduledTasks[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < maxthreads; i++) {
        pthread_join(t->threads[i], NULL);
    }
}

/**
 * Add content to a node group
 * @param ng Pointer to node group
 * @param content Content to add
 */
void addContent(NodeGroup* ng, void* content) {
    Node* current = ng->head;
    while (current->next) {
        current = current->next;
    }
    current->next = malloc(sizeof(Node));
    current->next->content = content;
    current->next->next = NULL;
}