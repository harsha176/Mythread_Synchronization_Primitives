/*
 * This program implements the mythread_cond_init(),mythread_cond_destroy(), mythread_cond_wait(), mythread_cond_signal(),mythread_cond_broadcast()
 */
#include "mymutex.h"
#include "myatomic.h"
#include "mycond.h"
#include "futex.h"
#include "mythread.h"
#include<stdlib.h>
#include<stdio.h>

//Conditional variable control block
ccb_t **ccb_table = NULL;
//Number of conditional variables
int nr_conditional_variables = 0;

/*
 * This function initializes the conditional variable referenced by cond
 * Pre:A reference to a uninitialized variable of type mythread_cond_t 
 * Post:An entry in the Conditional variable control block and is initialized
 * Returns: -1 on failure
 * 	     0 on success
 */
int mythread_cond_init(mythread_cond_t * cond,
		       const mythread_condattr_t * attr)
{
    // Preventing race conditions while updating ccb table and allocating memory
    mythread_enter_kernel();
    ccb_table =
	(ccb_t **) realloc(ccb_table,
			   sizeof(ccb_t *) * (nr_conditional_variables +
					      1));
    if (!ccb_table) {
	perror("Failed to extend conditional variable table\n");
	return -1;
    }
    ccb_table[nr_conditional_variables] = (ccb_t *) malloc(sizeof(ccb_t));
    if (!ccb_table[nr_conditional_variables]) {
	perror
	    ("Failed to allocate memory for the new conditional variable\n");
	return -1;
    }
    // Initialize thread control block resources
    *cond = (mythread_cond_t) nr_conditional_variables;
    futex_init(&ccb_table[nr_conditional_variables]->mutex, 1);
    //ccb_table[nr_conditional_variables]-> q = NULL;
    nr_conditional_variables++;
    mythread_leave_kernel();
    return 0;
}

/*
 * This function destroys the conditional variable referenced by cond
 * Pre: A valid conditional varaible
 * Post: The referenced conditional variable is removed from the Conditional Control Variable block
 * Returns: -1 on failure
 * 	     0 on success
 */
int mythread_cond_destroy(mythread_cond_t* cond)
{
    mythread_enter_kernel();
    if (ccb_table != NULL && cond != NULL && ccb_table[*cond] != NULL) {
	free(ccb_table[*cond]);
	ccb_table[*cond] = NULL;
	*cond = -1;
	mythread_leave_kernel();
	return 0;
    } else {
	mythread_leave_kernel();
	return -1;
    }
}

/*
 * This function is used to block on a conditional variable referenced by cond. The attribute referenced by mutex should already be blocked
 * Pre:A conditional variable , a locked mutex
 * Post:The calling thread will be blocked. Upon return mutex has been locked and owned by calling thread
 * Returns:  1 on failure
 * 	     0 on success
 */
int mythread_cond_wait(mythread_cond_t * cond, mythread_mutex_t * mutex)
{
    mythread_enter_kernel();
    mythread_block_phase1(&ccb_table[*cond]->q, 0);

    // Inorder to ensure there will be no signal in between these operations
    // and therefore prevent signal loss futex has been used.
    if (futex_down(&ccb_table[*cond]->mutex)) {
	perror("futex_down failed\n");
	return 1;
    }

    // release lock after phase 1
    if (mythread_mutex_unlock(mutex)) {
	perror("Failed to release lock\n");
	return 1;
    }
    // in order to ensure signal is not received when the thread is about to suspend we need to perform futex up 
    // and block phase 2 atomically 
    mythread_enter_kernel();
    if (futex_up(&ccb_table[*cond]->mutex)) {
	perror("Failed to futex_up\n");
	return 1;
    }
    mythread_block_phase2();

    // Acquire mutex lock before returning from conditional wait
    if (mythread_mutex_lock(mutex)) {
	perror("Failed to acquire lock");
	return 1;
    }
    return 0;
}
/*
 * This function unblocks one of the threads blocked on a conditional variable
 * The scheduling policy uses FIFO
 * Pre:A conditional variable on which threads are blocked
 * Post:One thread which was blocked on the conditional variable is unblocked
 * Returns:  1 on failure
 * 	     0 on success
 */
int mythread_cond_signal(mythread_cond_t * cond)
{
    /*
     * Wait if there is any thread is executing conditional wait
     * This is used to ensure no signal is lost 
     * while any thread is executing condtional wait code.  
     */
    if (futex_down(&ccb_table[*cond]->mutex)) {
	perror("failed to do futex_down");
	return 1;
    }

    /*
     * Dequeue and resume the thread from blocked queue
     */  
    mythread_enter_kernel();
    if(ccb_table[*cond]->q != NULL) {
          mythread_unblock(&ccb_table[*cond]->q, 0);
    } else {
          mythread_leave_kernel();
    }
    /*
     * Resume any thread about to execute conditional wait
     */  
    if (futex_up(&ccb_table[*cond]->mutex)) {
	perror("failed to do futex up\n");
	return 1;
    }
    return 0;
}

/*
 * This function unblocks all the threads which are blocked on the conditional variable referenced by cond
 * Pre:A conditional variable on which threads are blocked
 * Post: All threads which are blocked on the conditional variable are unblocked
 * Returns:  1 on failure
 * 	     0 on success
 */
int mythread_cond_broadcast(mythread_cond_t * cond)
{
    if (futex_down(&ccb_table[*cond]->mutex)) {
	perror("failed to do futex_down\n");
	return 1;
    }
    /*
     * Remove each thread from head of the queue and wake up all the blocked threads.
     */    
    mythread_enter_kernel();
    while (ccb_table[*cond]->q != NULL) {
	mythread_unblock(&ccb_table[*cond]->q, 0);
        mythread_enter_kernel();
    }
    mythread_leave_kernel();
    /*
     * Resume any thread that is about to execute conditional wait
     */       
    if (futex_up(&ccb_table[*cond]->mutex)) {
	perror("failed to do futex_up\n");
	return 1;
    }
    return 0;
}
