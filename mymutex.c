/*
 * file: mymutex.c
 */
#include <stdlib.h>
#include <stdio.h>
#include "mymutex.h"
#include "mythread.h"
#include "myatomic.h"

#define BLOCKED 16         // status to denote blocked threads
#define MAX_RETRIES 100    // Maximum number of retries allowed   

mcb_t **mcb_table = NULL;  // mutex table is initialized to NULL
int nr_mutexes = 0;        // number of mutexes intialized

static int is_initialized = 0;  // flag indicating the status of mythread library initialization. 
                                // O implies its not initialized
/*
 * Dummy method used to initialize thread library
 */
void* foo(void* arg) {
   return NULL;
}

/*
 * Initializes thread library by launching dummy thread
 */
void initialize() {
   mythread_t foo_tid;
   mythread_create(&foo_tid, NULL, foo, NULL);
}


/*
 * This method initializes mutex. A pointer to mutex handle needs to be passed.
 * mutex attribute is not used and NULL can be passed.
 *
 * Pre: valid pointer to mutex handle
 * Post: mutex initialized appropriate resources are created and initialized.
 *       Returns mutex handle after successful initialization.
 *
 */
int mythread_mutex_init(mythread_mutex_t * mutex,
			const mythread_mutexattr_t * attr)
{
    // Initialize mythread library if not initialized. 
    // This is needed before any mythread_enter_kernel() or mythread_leave_kernel() is called.
    if(!is_initialized) {
       initialize();
       is_initialized = 1;
    }

    /*
     * updating mcb_table so it needs to be protected from other threads and prevent race conditions
     * It is protected by call to mythread_enter_kernel() 
     */ 
    mythread_enter_kernel();
    mcb_table =
	(mcb_t**) realloc(mcb_table, sizeof(mcb_t*)*(nr_mutexes + 1));
    if (!mcb_table) {
	perror("Failed to extened mutex table");
	return 0;
    }
    mcb_table[nr_mutexes] = (mcb_t*)malloc(sizeof(mcb_t));
    
    if(!mcb_table[nr_mutexes]) {
	perror("Failed to allocate memory");
	return 0;
    }

    // Initialize mutex control data structures
    *mutex = (mythread_mutex_t)nr_mutexes;
    mcb_table[nr_mutexes]->mid = *mutex;
    mcb_table[nr_mutexes]->lock = 0;
    mcb_table[nr_mutexes]->block_queue = NULL;

    // Update thread count for the mutexes
    nr_mutexes++;

    mythread_leave_kernel();
    return *mutex;
}

/*
 * This method removes the resources associated with mutex handle from the system
 * Pre: Initialized mutex
 * Post: mutex handle is invalidated and resources associated with mutex handle are removed
 *
 */
int mythread_mutex_destroy(mythread_mutex_t * mutex)
{

   // To prevent race conditions from updating mcb_table and while allocating memory.
   mythread_enter_kernel();
    if (mutex != NULL && *mutex > 0 && mcb_table[*mutex] != NULL) {
	free(mcb_table[*mutex]);   // free the resource
	mcb_table[*mutex] = NULL;  
	*mutex = -1;               // invalidate mutex id
        mythread_leave_kernel();
	return 0;
    } else {
        mythread_leave_kernel();
	return -1;
    }
}

/*
 * This method provides lock construct and if lock is free it returns otherwise it will be in busy waiting for MAX_RETRIES 
 * and then eventually suspends on the futex.
 * Pre: Initialized mutex
 * Post: acquires lock on return
 */
int mythread_mutex_lock(mythread_mutex_t * mutex)
{
    mythread_enter_kernel();
    if (mutex != NULL && *mutex >= 0 && mcb_table != NULL && mcb_table[*mutex] != NULL) {
	int retry_count = 0;
	int *lock = &(mcb_table[*mutex]->lock);
        mythread_leave_kernel();
        // keep threads in busy spinning till it reaches MAX_RETRIES
	while (1 /*retry_count < MAX_RETRIES*/) {
            // keep threads in busy spinning till they reach MAX_RETRIES
	    while (*lock == 1 && retry_count < MAX_RETRIES);
		retry_count++;
            // break from busy spinning if retry count reaches MAX_RETRIES
	    if (retry_count >= MAX_RETRIES) {
		break;
	    }
            // compare and swap procedure is used to implement test and set lock
            // 0 implies free
            // 1 implies busy
	    if (compare_and_swap(lock, 1, 0) == 0) {
		return 0;
	    }
	}
	// suspend threads if they retry count exceeds MAX_RETRY
	mythread_enter_kernel();
	mythread_block(&(mcb_table[*mutex]->block_queue), BLOCKED);
	return 0;
    } else {
        mythread_leave_kernel();
	return -1;
    }
}

/*
 * This method provides unlock construct. On calling this procedure a thread is returns from lock procedure 
 * if there is any thread waiting to acquire lock will be released otherwise lock is set to free state.
 * Pre: Intialized mutex and has acquired lock
 * Post: lock is released and returns status of the mutex
 */
int mythread_mutex_unlock(mythread_mutex_t * mutex)
{
    mythread_enter_kernel();
    if (mcb_table[*mutex]->block_queue == NULL) {
	mcb_table[*mutex]->lock = 0;
    } else {
	mythread_unblock(&(mcb_table[*mutex]->block_queue), BLOCKED);
    }
    mythread_leave_kernel();
    return 0;
}
