#include "multiThreaded.h"

// Variables globales pour le timing
clock_t thread_start, thread_stop;

// Fonction exécutée par chaque thread pour traiter les tâches
void* doallTasks(void* arg) {
    NodeGroup* schedule = (NodeGroup*)arg;
    if (!schedule->head) return NULL;
    
    Node* current = schedule->head;
    Node* temp = NULL;
    int tasks_executed = 0;
    
    while (current) {
        if (current->content) {
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

// Initialisation d'un groupe de nœuds
void initNodeGroup(NodeGroup* ng) {
    ng->head = malloc(sizeof(Node));
    ng->head->content = NULL;
    ng->head->next = NULL;
}

// Configuration du système de threads
Threads* setupThreads() {
    Threads* newThreads = malloc(sizeof(Threads));
    newThreads->doall = doallTasks;
    
    for (int i = 0; i < maxthreads; i++) {
        newThreads->occupency[i] = 0;
        // Initialisation du premier nœud
        newThreads->scheduledTasks[i].head = malloc(sizeof(Node));
        newThreads->scheduledTasks[i].head->content = NULL;
        newThreads->scheduledTasks[i].head->next = NULL;
    }
    
    return newThreads;
}

// Ajoute une tâche à un groupe
void addTaskToGroup(NodeGroup g, Task* task) {
    Node* current = g.head;
    while (current->next) {
        current = current->next;
    }
    Node* new = malloc(sizeof(Node));
    new->next = NULL;
    new->content = (void*)task;
    current->next = new;
    return;
}

// Ajoute une tâche au thread le moins chargé
void addTaskInThreads(Threads* t, void (*task)(void* param), void* data) {
    if (!t) return;
    int min = t->occupency[0];
    int slot = 0;
    for (int i = 0; i < maxthreads; i++) {
        if (t->occupency[i] < min) {
            min = t->occupency[i];
            slot = i;
        }
    }
    Task* ntsk = malloc(sizeof(Task));
    ntsk->task = task;
    ntsk->data = data;
    addTaskToGroup(t->scheduledTasks[slot], ntsk);
    t->occupency[slot]++;
}

// Exécute toutes les tâches planifiées sur les threads
void handleThreads(Threads* t) {
    if (!t) return;
    
    // Création des threads
    for (int i = 0; i < maxthreads; i++) {
        pthread_create(&t->threads[i], NULL, t->doall, (void*)&t->scheduledTasks[i]);
    }
    
    // Attente de la fin des threads
    for (int i = 0; i < maxthreads; i++) {
        pthread_join(t->threads[i], NULL);
    }
}

// Fonction utilitaire pour ajouter du contenu à un NodeGroup
void addContent(NodeGroup* ng, void* content) {
    Node* current = ng->head;
    while (current->next) {
        current = current->next;
    }
    current->next = malloc(sizeof(Node));
    current->next->content = content;
    current->next->next = NULL;
}