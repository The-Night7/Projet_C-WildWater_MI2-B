#ifndef MULTITHREADED_H
#define MULTITHREADED_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Configuration du nombre de threads
#define maxthreads 4

// Variables pour mesurer les performances
extern clock_t thread_start, thread_stop;

// Structures de base pour la gestion des threads
typedef struct node {
    void* content;
    struct node* next;
} Node;

typedef struct {
    Node* head;
} NodeGroup;

// Structure pour les tâches
typedef struct {
    void* data;
    void (*task)(void* param);
} Task;

// Structure principale pour gérer les threads
typedef struct {
    int occupency[maxthreads];
    NodeGroup scheduledTasks[maxthreads];
    pthread_t threads[maxthreads];
    void* (*doall)(void* self);
} Threads;

// Fonctions de gestion des threads
void initNodeGroup(NodeGroup* ng);
void* doallTasks(void* arg);
void addTaskToGroup(NodeGroup g, Task* task);
void addTaskInThreads(Threads* t, void (*task)(void* param), void* data);
void handleThreads(Threads* t);
Threads* setupThreads();
void addContent(NodeGroup* ng, void* content);

#endif /* MULTITHREADED_H */