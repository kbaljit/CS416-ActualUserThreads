// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	09/23/2017

// name: Baljit Kaur, Surekha Gurung, Krishna Patel
// username of iLab:
// iLab Server: top.cs.rutgers.edu

#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE

/* To use real pthread Library in Benchmark, you have to comment the USE_MY_PTHREAD macro */
#define USE_MY_PTHREAD 1

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>   
#include <signal.h>    
#include <unistd.h>       
#include <sys/time.h>
#include <ucontext.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define MAX_THREADS2 200
/* STRUCTS */

typedef struct {
	// Define any fields you might need inside here.
} my_pthread_attr_t;

typedef struct my_pthread_t {
	int thread_id; 
	int dead; 
	ucontext_t ctx;
	void* args;
	struct my_pthread_t* next_thread;
} my_pthread_t; 

typedef struct node {

	my_pthread_t* thread;   
	int NodePrio;          // priority
	double time2;         
	int action; // 0=nothing; 1=yield; 2=exit; 3=join; 
	ucontext_t* ctx;       
	time_t create;
	my_pthread_t* joinThread;    
	struct node* next;         

} node;

typedef struct queue {
	int prio;
	node* head;
	node* rear;
} queue;

typedef struct listOfThreads {
	my_pthread_t* head;
} listOfThreads;  

typedef struct listOfNodes {
	node* head;
} listOfNodes;   

typedef struct info {

	int calls;     
	int numThreads; 
	node* curr_node;  

	listOfNodes* join_list;  
	listOfThreads* all_threads;  // threads

	ucontext_t* sched;  
	struct itimerval* timer;  

	queue* promoQueue[4]; 
	queue* runQueue[5]; 

	// time_t begin;

} info;

/* globals */
static info* data = NULL;

//my_pthread_t* current_thread = NULL;

static int initialized = 0;
//my_pthread_t * curr_LowThread = 0;
	
//static my_pthread_t * thread_list[MAX_THREADS2];

//static my_pthread_t * run_list[MAX_THREADS2];
//static my_pthread_t * lowP_list[MAX_THREADS2];

/* function declarations */

void swap();
void check();
void scheduler();
void maintenance();
void initQueues();
void initLists();
void initContext();
void ctxHandler();
void alrmHandle(int signum);
node* caluclatePlace(int place, node* tempNode);
//node* dequeue(int level, int choose);
double findTime(clock_t timeNow, node* temp);
void doTheRest(int* temp, node* tempNode);
node* dequeue(int level, int choose, queue* QTemp);
void createMain(ucontext_t* main_ctx);
void enqueue(int level, node* insertNode);
void insertByTime(queue* q, node* newNode);
void remove2(int* temp, int lev, node* tempNode, int j);
void change(int* temp, int lev, node* tempNode, int j);
node* list_funcs(int task, listOfNodes* tempList, my_pthread_t* tempThread);

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);
/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();
/* terminate a thread */
void my_pthread_exit(void *value_ptr);
/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);


// Mutex Decelarations:

typedef struct my_pthread_mutex_t {
	int flag;
} my_pthread_mutex_t;

typedef int my_pthread_mutexattr_t;

int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#ifdef USE_MY_PTHREAD
#define pthread_t my_pthread_t
#define pthread_mutex_t my_pthread_mutex_t
#define pthread_create my_pthread_create
#define pthread_exit my_pthread_exit
#define pthread_join my_pthread_join
#define pthread_mutex_init my_pthread_mutex_init
#define pthread_mutex_lock my_pthread_mutex_lock
#define pthread_mutex_unlock my_pthread_mutex_unlock
#define pthread_mutex_destroy my_pthread_mutex_destroy
#endif

#endif















