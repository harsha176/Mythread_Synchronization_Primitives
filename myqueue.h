/*
 * myqueue.h -- interface for queue ops
 */
#ifndef MY_QUEUE_H 
#define MY_QUEUE_H 1

#include <malloc.h>
#include <stdio.h>

typedef struct mythread_queue {
  void *item;
  struct mythread_queue *prev, *next;
} *mythread_queue_t;

#endif
