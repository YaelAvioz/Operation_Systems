#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "osqueue.h"
#include <string.h>

typedef enum { RUNNING, FORCE_STOP, WAIT_STOP } state;

typedef struct thread_pool {
  pthread_t *threads;
  OSQueue *queue;
  int numThreads;
  pthread_mutex_t lock;
  pthread_cond_t notify;
  state state;
} ThreadPool;


typedef struct {
    void (*func)(void *);
    void *args;
} Task;

/**
 * Print error to stderr (fd number 2)
 */
void PrintError();

/**
 * Create a threadpool
 * @param numOfThreads number of threads in the pool
 * @return a pointer to the threadpool
 */
ThreadPool *tpCreate(int numOfThreads);

/**
 * Destroy the threadpool and free memory
 * @param pool a pointer to the threadpool
 * @param shouldWaitForTasks 0 if should wait, otherwise don't wait.
 */
void tpDestroy(ThreadPool *pool, int shouldWaitForTasks);

/**
 * Insert a new task to the queue
 * @param pool a pointer to the threadpool
 * @param computeFunc the function to compute
 * @param param the parameters for the function
 * @return in success 0, in failure a different number
 */
int tpInsertTask(ThreadPool *pool, void (*computeFunc)(void *), void *param);

/**
 * The function executed by the thread
 * @param param a pointer to the threadpool
 * @return the result of the thread computation.
 */
static void *ThreadFunction(void *param);

#endif