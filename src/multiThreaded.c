#include "multiThreaded.h"

// Variables globales pour le timing
clock_t thread_start, thread_stop;

// Essential for a thread to execute a scheduled task
void * doallTasks(void * arg){
	fprintf(stderr, "Thread %lu started\n", (unsigned long)pthread_self());
	NodeGroup * schedule = (NodeGroup*)arg;
	if (!schedule->head)return NULL;
	Node *current = schedule->head;
	Node *temp =NULL;
	int tasks_executed = 0;
	while(current){
		if (current->content){
			((Task*)current->content)->task(((Task*)current->content)->data);
			tasks_executed++;
		}
		temp = current;
		schedule -> head = current->next;
		current = current->next;
		free(temp);
	}
	fprintf(stderr, "Thread %lu completed %d tasks\n", (unsigned long)pthread_self(), tasks_executed);
	return NULL;
}

// Thread creation and handling
void initNodeGroup(NodeGroup *ng)
{
	ng->head = malloc(sizeof(Node));
	ng->head -> content = NULL;
	ng->head -> next    = NULL;
}

Threads * setupThreads()
{
	fprintf(stderr, "Setting up %d threads\n", maxthreads);
	Threads * newThreads = malloc(sizeof(Threads));
	newThreads -> doall =  doallTasks;
	
	for (int i =0;i<maxthreads;i++){
		newThreads->occupency[i]=0;
		// Setting up the first node;
		newThreads->scheduledTasks[i].head= malloc(sizeof(Node));
		newThreads->scheduledTasks[i].head->content = NULL;
		newThreads->scheduledTasks[i].head->next = NULL;
	}
	fprintf(stderr, "Thread system initialized successfully\n");
	return newThreads;
}

void addTaskToGroup(NodeGroup g,Task * task)
{
	Node *current =g.head;
	while (current->next){
		current = current->next;
	}
	Node *new=malloc(sizeof(Node));
	new-> next =NULL;
	new->content = (void*)task;
	current-> next = new;
	return;
}

void addTaskInThreads(Threads * t, void (*task)(void*param),void*data)
{
	if(!t)return;
	int min = *t->occupency;
	int slot = 0;
	for	(int i = 0; i<maxthreads;i++){
		if (t->occupency[i]<min){
			min = t->occupency[i];
			slot = i;
		}
	}
	Task * ntsk = malloc(sizeof(Task));
	ntsk -> task = task;
	ntsk -> data = data;
	addTaskToGroup(t->scheduledTasks[slot],ntsk);
	t->occupency[slot]++;
}

// To execute the treads scheduled for a thread
void handleThreads(Threads* t)
{
	fprintf(stderr, "Starting %d threads\n", maxthreads);
	if(!t)return;
	
	// Count total tasks
	int total_tasks = 0;
	for (int i = 0; i < maxthreads; i++) {
		Node *current = t->scheduledTasks[i].head;
		while (current->next) {
			total_tasks++;
			current = current->next;
		}
	}
	fprintf(stderr, "Total tasks scheduled: %d\n", total_tasks);
	
	// Create threads
	for (int i =0; i<maxthreads;i++){
		fprintf(stderr, "Creating thread %d with %d tasks\n", i, t->occupency[i]);
		pthread_create(&t->threads[i],NULL,t->doall,(void*)&t->scheduledTasks[i]);
	}
	
	// Wait for threads to complete
	for (int i = 0 ; i<maxthreads;i++){
		pthread_join(t->threads[i],NULL);
		fprintf(stderr, "Thread %d completed\n", i);
	}
	
	fprintf(stderr, "All threads completed\n");
}

// Fonction utilitaire pour ajouter du contenu à un NodeGroup
void addContent(NodeGroup * ng,void*content){
	Node * current = (Node*)ng -> head;
	while(current->next){
		current = current->next;
	}
	current-> next = malloc(sizeof(Node));
	current-> next-> content = content;
	current-> next-> next = NULL;
}