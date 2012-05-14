#ifndef MYTHREAD_COND_H
#define MYTHREAD_COND_H 1
#include "myqueue.h"
#include "futex.h"
#include "mymutex.h"
typedef int mythread_cond_t;

typedef struct _conditional_variable
{
int cond_id;
struct futex mutex;
mythread_queue_t q;
}ccb_t;

typedef struct _mythread_condattr_t{
int value;
}mythread_condattr_t;
/*
 * Initializing the conditional varialbe
 */
int mythread_cond_init(mythread_cond_t *cond,const mythread_condattr_t *attr);
/*
 *Destroy the caonditional variable specified by cond 
 */
int mythread_cond_destroy(mythread_cond_t *cond);
/*
 * The calling thread blocks on the conditional variable cond*/
int mythread_cond_wait(mythread_cond_t *cond, mythread_mutex_t *mutex);
/*
 * Unblocks one of the threads which is blocked on cond
 */
int mythread_cond_signal(mythread_cond_t *cond);
/*
 * Unblocks all the threads which are blocked on cond
 */
int mythread_cond_broadcast(mythread_cond_t *cond);
#endif
