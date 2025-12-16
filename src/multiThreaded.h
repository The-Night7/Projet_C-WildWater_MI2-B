#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
// Defining essential elements

// Réduire temporairement à 1 thread pour tester les performances
#define maxthreads 1

// Déclaration des variables de temps pour le timing
extern clock_t thread_start, thread_stop;

// predefining the structures ig
typedef struct node node;

// DEFINING IMPORTANT SHI

// basic and common types

typedef struct node{
	void * content;
	node * next;
}Node;

typedef struct{
	Node * head;
}NodeGroup;

// types used for scheduling tasks in threads;

typedef struct {
	void * data;
	void (*task)(void *param);
}Task;

typedef struct {
	int occupency[maxthreads];
	NodeGroup scheduledTasks[maxthreads]; //a NodeGroup
	pthread_t threads[maxthreads];
	void *(*doall)(void * self);
}Threads;

void initNodeGroup(NodeGroup *ng);

void * doallTasks(void*arg);
void addTaskToGroup(NodeGroup g, Task * task);
void addTaskInThreads(Threads * t, void (*task)(void*param),void*data);
void handleThreads(Threads* t);

Threads * setupThreads();

// Fonction utilitaire pour ajouter du contenu à un NodeGroup
void addContent(NodeGroup * ng, void * content);