/* 
 * file: mymutex.h
 */
#ifndef MYTHREAD_MUTEX_H
#define MYTHREAD_MUTEX_H 1
#include "myqueue.h"

/*
 * This defines mutex handle and acts a mutex identifier for library users to perform mutex operations
 */
typedef int mythread_mutex_t;

/*
 * Mutex control block is used to store mutex properties and its state.
 */
typedef struct _mutex_control_block {
   int lock;                        // state of the lock; 0 indicates its free and 1 for busy
   mythread_mutex_t mid;	    // mutex identifier
   mythread_queue_t block_queue;    // block queue on which threads will wait when mutex is locked
}mcb_t;

/*
 * Mutex attributes identifier
 *
 * This not used and is only added for consistency with pthread interface
 */
typedef struct _mythread_mutexattr_t {
   int value;
}mythread_mutexattr_t;


/*
 * This method initializes mutex and returns the id of mutex
 * Pre: pointer to mutex to store its id after initialization
 * Post: mutex is intialized and returns the id of the mutex
 */
int mythread_mutex_init(mythread_mutex_t *mutex, const mythread_mutexattr_t *attr);

/*
 * This method locks and blocks any other threads if lock is busy and number of retries exceed MAX_RETRY count that is 100.
 * Pre: mutex is initialized
 * Post: If lock is free then it sets lock state to busy and returns otherwise the thread is blocked till lock becomes free 
 *       and thread gets a chance.
 */
int mythread_mutex_lock(mythread_mutex_t *mutex);

/*
 * This method releases lock and wakes up any other threads. Priority is given for the threads that are blocked on mutex
 * and only when blocked threads released busy waiting threads get chance.
 *
 * Pre: mutex is initialized
 * Post: unlocks mutex and returns. If there are any blocked threads they are freed
 */
int mythread_mutex_unlock(mythread_mutex_t *mutex);

/*
 * This method destroys mutex
 * 
 * Pre: mutex is initialized
 * Post: All the resources associated with the mutex are freed
 */
int mythread_mutex_destroy(mythread_mutex_t *mutex);
#endif
