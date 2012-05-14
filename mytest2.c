#include<stdio.h>
#include<unistd.h>
#include "mymutex.h"
#include "mycond.h"
#include"mythread.h"

int Aarrived =1;

mythread_mutex_t lock; // = PTHREAD_MUTEX_INITIALIZER;
mythread_cond_t hasaArrived; // = PTHREAD_COND_INITIALIZER;

void * func_A(void *arg)
{

	sleep(1);
	mythread_mutex_lock(&lock);
	Aarrived = 0;
	printf("A arrived\n");
	mythread_cond_signal(&hasaArrived);
	mythread_mutex_unlock(&lock);
	mythread_exit(NULL);
}

void * func_B(void *arg)
{
	mythread_mutex_lock(&lock);
	while(Aarrived) {
	    printf("B: A has not arrived yet. waiting\n");
		mythread_cond_wait(&hasaArrived, &lock);
	}
    printf("B Arrived\n");
	mythread_mutex_unlock(&lock);
	mythread_exit(NULL);
}

void* f (void* arg) {
    return NULL;
}
 
int main()
{
 
        mythread_setconcurrency(2);
        mythread_t dummy;
        mythread_create(&dummy, NULL, f, NULL);

        mythread_mutex_init(&lock, NULL);
        mythread_cond_init(&hasaArrived, NULL);

	mythread_t t1, t2;
	mythread_create(&t1, NULL, func_A, NULL);
	mythread_create(&t2, NULL, func_B, NULL);

	mythread_exit(NULL);
}

