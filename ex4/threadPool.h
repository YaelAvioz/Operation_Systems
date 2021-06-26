//Yael Avioz 207237421

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
  int threadsNumber;
  pthread_mutex_t lock;
  pthread_cond_t notify;
  state state;
} ThreadPool;

typedef struct {
    void (*func)(void *);
    void *args;
} Task;

ThreadPool *tpCreate(int numOfThreads);

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc)(void*), void* param);

static void* startRoutine(void* param);

#endif
