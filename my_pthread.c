// File:    my_pthread.c
// Author:  Yujie REN
// Date:    09/23/2017

// name: Baljit Kaur, Surekha Gurung, Krishna Patel
// username of iLab:
// iLab Server: top.cs.rutgers.edu

#include "my_pthread_t.h"

#define STACK 64000

void check() {
    if( data->calls >= 101 ) {
        data->calls = 0;
        
        int *temp = malloc( 4*sizeof(int) );
        node* tempNode = malloc( sizeof(node) );
        int i;

        for( i=0; i<4; i++ ) {
            temp[i] = 0;
            tempNode = caluclatePlace(i, tempNode);
            temp[i]++;
        }

        doTheRest(temp, tempNode);

        free(temp);
        temp = NULL;
    }
}

void remove2(int* temp, int lev, node* tempNode, int j) {
    int k;
    for(k=0; k<temp[lev]/(lev+2); k++) {
        tempNode = dequeue(lev, 0, data->promoQueue[lev]);
        if(tempNode != NULL) {
            enqueue(j-1, tempNode);
        }
    }
}

void change(int* temp, int lev, node* tempNode, int j) {
    int k;
    for(k= (temp[lev]/(lev+2)) * (lev+1); k<temp[lev]; k++) {
        tempNode = dequeue(lev, 0, data->promoQueue[lev]);
        if( tempNode != NULL) {
            enqueue(j-1, tempNode);
        }
    }
}
void doTheRest(int* temp, node* tempNode) {
    int i=0; int j=0; int k;
    for(i=0; i<4; i++) {
        for(j=i+2; j>0; j--) {
            if(j>1) {
                remove2(temp, i, tempNode, j);
            } else {
                change(temp, i, tempNode, j);
            }
        }
    }
}

double findTime(clock_t timeNow, node* temp) {
    double x = timeNow - temp->time2;
    x = x/CLOCKS_PER_SEC;
    return x;
}

node* caluclatePlace(int place, node* tempNode) {
    //node* tempNode;
    clock_t timeNow = clock();

    for(tempNode = dequeue(place+1, 1, data->runQueue[place+1]); tempNode!=NULL; tempNode = dequeue(place+1, 1, data->runQueue[place+1])) {
        tempNode->time2 = findTime(timeNow, tempNode);
        node* prevNode=NULL; node* pointer;

        if( data->promoQueue[place]->head == NULL ) {
            data->promoQueue[place]->head = tempNode;
            data->promoQueue[place]->rear = tempNode;
            data->promoQueue[place]->head->next = NULL;
            data->promoQueue[place]->head->next = NULL;
            return;
        }

        while(pointer!=NULL) {
            if( tempNode->time2 > pointer->time2 ) {
                if( prevNode == NULL ) {
                    tempNode->next = pointer;
                    data->promoQueue[place]->head = tempNode;
                }
                tempNode->next = pointer;
                prevNode->next = tempNode;
            }
            prevNode = pointer;
        }

        data->promoQueue[place]->rear->next = tempNode;
        data->promoQueue[place]->rear = tempNode;
        data->promoQueue[place]->rear->next = NULL;
    }
    return tempNode;
}

void swap() {
    //printf("in swap\n");
    node* temp = NULL;
    
    temp = data->curr_node;
   // printf("in swap2\n");

    node* next = list_funcs(2, data->join_list, temp->thread);  

    if( next == NULL ) { return; }

    // printf("in swap3\n");

    data->curr_node = next;
    data->timer->it_value.tv_usec = 100*1000* (data->curr_node->NodePrio + 1);
    setitimer(ITIMER_REAL, data->timer, NULL);

    // printf("in swap4\n");
    if( data->curr_node->thread->thread_id != temp->thread->thread_id ) {
        swapcontext(temp->ctx, data->curr_node->ctx);
    }

}

void scheduler() {
    //printf("in scheduler????\n");
        //stop(data->timer);
    struct itimerval zero;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &zero, data->timer);
    data->timer = data->timer;

    data->calls++;

    /*int x = data->calls % 10;

    if( x==0 ) {
        double k = ( (double) (clock() - data->begin)/ CLOCKS_PER_SEC );
        printf("runtime of program is : %f\n", k);
    } */

    check();

    if( data->curr_node->action == 3 ) {    /// thread is JOINING
        //printf("in join cond\n");
            // thread must wait for other thread to finish executing
        data->curr_node->next = data->join_list->head;
        data->join_list->head = data->curr_node;
    }

    else if( data->curr_node->action == 2 ) {    /// thread is EXITING
        //printf("in exit cond\n");
        my_pthread_t* temp2 = data->curr_node->thread;
        my_pthread_t* temp3 = data->all_threads->head;  // ptr

        int k=0;

            // change status to dead
        while( temp3!=NULL ) {
            k++;
            if( temp3->thread_id == data->curr_node->thread->thread_id) {
                temp3->dead = 1;
                //printf("k is: %d\n", k);
                break;
            }
            temp3 = temp3->next_thread;
        }
            // change status
        /*for(temp3 = data->all_threads->head; temp3!=NULL; temp3 = temp3->next_thread) {
            k++;
            if( temp3->thread_id == temp2->thread_id ) {
                printf("k is %d\n", k);
                temp3->dead = 1;
                break;
            }
        }*/
        node* temp_node = data->join_list->head;
        //node* add_node;

        while(temp_node != NULL) {
            //printf("this loop?\n");
            if( temp_node->joinThread->thread_id == data->curr_node->thread->thread_id) {
                //printf("this one also?\n");
                node* add_node =  list_funcs(1, data->join_list, temp_node->thread);
                enqueue(add_node->NodePrio, add_node);
                temp_node = data->join_list->head;
            } else {
                temp_node = temp_node->next;
            }
        }
    } 

    else if( data->curr_node->action == 1 ) {      /// thread is YIELDING
       // printf("in yield cond\n");
        int new_prio = data->curr_node->NodePrio;
        enqueue(new_prio, data->curr_node);
        data->curr_node->action = 0;
    } 

    else {      // node needs to be enqueued to lower prio queue
        int new_prio = data->curr_node->NodePrio;
        if( data->curr_node->NodePrio < 4 ) {
            new_prio++;
        }
        enqueue(new_prio, data->curr_node);
    } 

    swap();
}

node* list_funcs(int task, listOfNodes* tempList, my_pthread_t* tempThread) {
   // printf("in list_funcs, task: %d\n", task);
    if(task == 1) {
        node* pointer = tempList->head;
        node* prev = NULL;

        while( pointer!=NULL && tempThread->thread_id != pointer->thread->thread_id ) {
           // printf("in this hwile loop list_funcs\n");
                prev = pointer;
                pointer = pointer->next;
                //break;
            //pointer = pointer->next;
        }
        /*while( tempThread->thread_id!=pointer->thread->thread_id && pointer!=NULL ) {
            prev = pointer;
            pointer = pointer->next;
        }*/

        if( pointer!=NULL ) {
          //  printf("in this hwile loop list_funcs\n");
            if(pointer->thread->thread_id == tempThread->thread_id) {
                if( prev == NULL) {
                    node* final = pointer;
                    tempList->head = pointer->next;
                    final->next = NULL;
                    return final;
                } else {
                    prev->next = pointer->next;
                    pointer->next = NULL;
                    return pointer;
                }
            }
        }
        return NULL;
    } else {
        //printf("in list_funcs else\n");
        int track;
        for( track=0; track<5; track++ ) {
            //printf("in list_funcs track: %d\n", track);
            if( data->runQueue[track]->head != NULL ) {
              //  printf("in list_funcs got one %d\n", track);
                node* final = dequeue(track, 1, data->runQueue[track]);
               // printf("in list_funcs got result%d\n", track);
                return final;
            }
        }
        //printf("REturn null??\n");
        return NULL;
    }
}

void ctxHandler() {

        //stop(data->timer);
    struct itimerval zero;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &zero, data->timer);
    data->timer = data->timer;

    data->curr_node->action = 2;

        //resume(data->timer) :
    setitimer(ITIMER_REAL, data->timer, NULL);

    scheduler();
}

void alrmHandle(int signum) {
    data->timer->it_value.tv_usec = 0;
    scheduler();
}

void initQueues() {

    int i;
    for( i=0; i<5; i++ ) {
        data->runQueue[i] = (queue*) malloc( sizeof(queue) );
        
        data->runQueue[i]->rear = NULL;
        data->runQueue[i]->head = NULL;
        data->runQueue[i]->prio = i;

    }

    for( i=0; i<4; i++ ) {
        data->promoQueue[i] = (queue*) malloc( sizeof(queue) );
        data->promoQueue[i]->head = NULL;
        data->promoQueue[i]->rear = NULL;
        data->promoQueue[i]->prio = i;
    }

    return;
}

void initLists() {
    data->join_list = malloc( sizeof(listOfNodes) );
    data->join_list->head = NULL;

    data->all_threads = malloc( sizeof(listOfThreads) );
    data->all_threads->head = NULL;

    data->numThreads = 2;

    return;
}

void initContext() {
    ucontext_t* context= (ucontext_t*) malloc( sizeof(ucontext_t) );
    getcontext(context); 

    context->uc_stack.ss_sp = malloc(STACK);
    context->uc_stack.ss_size = STACK;
    makecontext(context, ctxHandler, 0);

    data->sched = context;
    data->calls = 0;

    return;
}

void createMain(ucontext_t* main_ctx) {
    my_pthread_t* mainThread = malloc( sizeof(my_pthread_t) );
    mainThread->thread_id = 1;
    mainThread->dead = 0;
    mainThread->next_thread = NULL;
    mainThread->args = NULL;


    node* Nodemain = (node*) malloc( sizeof(node) );
    Nodemain->thread = mainThread;
    Nodemain->ctx = main_ctx;
    Nodemain->next = NULL;
    Nodemain->create = clock();
    Nodemain->time2 = 0;
    Nodemain->NodePrio = 0;
    Nodemain->action = 0;
    Nodemain->joinThread = NULL;

    data->curr_node = Nodemain;
    return;

}

void enqueue(int level, node* insertNode) {
    queue* targetQueue = data->runQueue[level];

    if( data->runQueue[level]->head != NULL ) {
        data->runQueue[level]->rear->next = insertNode;
        data->runQueue[level]->rear = insertNode;
        data->runQueue[level]->rear->next = NULL;

        if( insertNode->action != 1 ) {
            insertNode->NodePrio = data->runQueue[level]->prio;
        }

    } else {
        data->runQueue[level]->head = insertNode;
        data->runQueue[level]->rear = insertNode;
        data->runQueue[level]->head->next = NULL;
        data->runQueue[level]->rear->next = NULL;
        insertNode->NodePrio = data->runQueue[level]->prio;
    }

}

node* dequeue(int level, int choose, queue* QTemp) {
    queue* targetQueue;

    if(choose == 1 ) {
        targetQueue = data->runQueue[level];
    } else {
        targetQueue = data->promoQueue[level];
    }

    node* first = QTemp->head;

    if( first==NULL ) { return NULL; } 

    node* findNode = first;
    QTemp->head = first->next;
    findNode->next = NULL;

    if( QTemp->head == NULL ) {
        QTemp->rear = NULL;
    }

    return findNode;
}

/************************** pthread functions *******************/
/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	
    //printf("hello from create");
	if( !initialized ) {
		
        data = (info*) malloc( sizeof(info) );

        data->curr_node = NULL;
		data->timer = (struct itimerval*) malloc( sizeof(struct itimerval));
        
        // data->begin = clock();

        initLists();
        initQueues();
        initContext();

        ucontext_t* main_ctx = malloc( sizeof(ucontext_t) );
        getcontext(main_ctx);

        createMain(main_ctx);

        signal(SIGALRM, alrmHandle);

        data->timer->it_value.tv_usec = 1000 * 100;

        // resume timer: 
        setitimer(ITIMER_REAL, data->timer, NULL);

        initialized = 1;

	}

        // stop timer: 
    struct itimerval zero;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &zero, data->timer);
    data->timer = data->timer;

    ucontext_t* ctx1 = (ucontext_t*) malloc( sizeof(ucontext_t) );

        // error allocating space for context
    if(ctx1 == NULL ) {
            //resume timer:
        setitimer(ITIMER_REAL, data->timer, NULL);
        return 1;
    }

        // initialize stack for context
    getcontext(ctx1);
    ctx1->uc_stack.ss_sp = (char*) malloc(STACK);
    ctx1->uc_stack.ss_size = STACK;
    ctx1->uc_link = data->sched;

    makecontext(ctx1, (void(*)(void))function, 1, arg);

    my_pthread_t* thread2 = thread;
    thread2->thread_id = data->numThreads;
    data->numThreads++;
    thread2->dead = 0;
    thread2->args = NULL;
    thread2->next_thread = NULL;

    thread2->next_thread = data->all_threads->head;
    data->all_threads->head = thread2;

    node* newNode = (node*) malloc( sizeof(node) );
    newNode->ctx = ctx1;
    newNode->next = NULL;
    newNode->thread = thread;
    newNode->time2 = 0;
    newNode->create = clock();
    newNode->action = 0;
    newNode->joinThread = NULL;
    newNode->NodePrio = 0;

    enqueue(0, newNode);

    //resume timer: 
    setitimer(ITIMER_REAL, data->timer, NULL);

    return 0;

}

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	//printf("hello from yield");
    if(!initialized) {
        return;
    }
        // stop timer:
    struct itimerval zero;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &zero, data->timer);
    data->timer = data->timer;

    data->curr_node->action = 1;

        //resume timer: 
    setitimer(ITIMER_REAL, data->timer, NULL);

    scheduler();

}

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
    //printf("hello from exit");
    if( !initialized ) { return; }

        // stop timer:
    struct itimerval zero;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &zero, data->timer);
    data->timer = data->timer;

    data->curr_node->action = 2;
    data->curr_node->thread->args = value_ptr;

        //resume timer: 
    setitimer(ITIMER_REAL, data->timer, NULL);
    scheduler();
}

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
   // printf("hello from join\n");
	if( !initialized ) { return 1; }

        // stop timer:
    struct itimerval zero;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &zero, data->timer);
    data->timer = data->timer;

    my_pthread_t* temp;
    temp = data->all_threads->head;

    while( temp != NULL ) {
    //    printf("while loop\n");
    //for(temp=data->all_threads->head; temp!=NULL; temp = temp->next_thread) {

        if( temp->thread_id == thread.thread_id) {

            if( temp->dead ) {
                if( value_ptr != NULL ) {
                    *value_ptr = temp->args;
                }
                //resume timer: 
                setitimer(ITIMER_REAL, data->timer, NULL);
                return 0;
            } else {
                data->curr_node->joinThread = temp;
                data->curr_node->action = 3;
                    //resume timer: 
                setitimer(ITIMER_REAL, data->timer, NULL);
                scheduler();
                    // stop timer:
                struct itimerval zero;
                zero.it_value.tv_usec = 0;
                setitimer(ITIMER_REAL, &zero, data->timer);
                data->timer = data->timer;

                if( value_ptr!= NULL) {
                    *value_ptr = temp->args;
                }
                    //resume timer: 
                setitimer(ITIMER_REAL, data->timer, NULL);
                return 0;
            }

            /*if(temp->dead && value_ptr!=NULL) {
                *value_ptr = temp->args;
                    //resume timer: 
                setitimer(ITIMER_REAL, data->timer, NULL);
                return 0;
            } 
            else if( temp->dead && value_ptr==NULL) {
                    //resume timer: 
                setitimer(ITIMER_REAL, data->timer, NULL);
                return 0;
            } */
        }
        temp = temp->next_thread;
    }
        //resume timer: 
    setitimer(ITIMER_REAL, data->timer, NULL);
    return 1;  
}


/**************************************** LOCKS DONE **********************************************/
/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	//printf("hello from mutex init");
    if( mutex != NULL ) {
		mutex->flag = 0;
		return 0;
	}
	return EINVAL;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	//printf("hello from mutex lock");
    if( mutex!=NULL ) {
 		while( __sync_lock_test_and_set(&(mutex->flag), 1)) {
 			my_pthread_yield();
 		}
 		return 0;
 	}
 	return EINVAL;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
    //printf("hello from mutex unlock");
	if( mutex!=NULL ) {
		__sync_synchronize();
		mutex->flag = 0;
		return 0;
	}
	return EINVAL;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
   // printf("hello from mutex destroy");
	if( mutex->flag == 0 ) {
		mutex = NULL;
	} else if( mutex->flag == 1 ) {
		//printf("flag is 1\n");
		return EBUSY;
	}
	//printf("about the return from destroy\n");
	return 0;
};

























