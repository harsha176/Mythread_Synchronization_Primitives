#include "mythread.h"
#include "mymutex.h"
#include "mycond.h"

#define	NR_THREADS       100

mythread_cond_t okToProceed;
mythread_mutex_t mutex;

void *runner(void *arg)
{
    int i;

    mythread_mutex_lock(&mutex);
    printf("Thread:Waiting for main thread to broadcast\n");
    fflush(stdout);
    mythread_cond_wait(&okToProceed, &mutex);
    sleep(0.2);
    printf("Thread:Out of conditonal wait and about to exit\n");
    fflush(stdout);
    mythread_mutex_unlock(&mutex);
    mythread_exit(NULL);
    return NULL;
}

int main()
{
    int i;
    mythread_t thread[NR_THREADS];
    mythread_setconcurrency(3);

    mythread_mutex_init(&mutex, NULL);
    mythread_cond_init(&okToProceed, NULL);

    for (i = 0; i < NR_THREADS; i++)
	mythread_create(&thread[i], NULL, runner, NULL);
    sleep(2);
    printf("Main: sending broadcast signal to all threads\n");
    fflush(stdout);
    mythread_cond_broadcast(&okToProceed);
    printf("Main: joining all threads\n");
    fflush(stdout);
    for (i = 0; i < NR_THREADS; i++)
	mythread_join(thread[i], NULL);
    printf("Main: about to exit\n");
    fflush(stdout);

    mythread_exit(NULL);

    mythread_cond_destroy(&okToProceed);
    mythread_mutex_destroy(&mutex);
    return 1;
}
