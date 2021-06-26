#include "threadPool.h"

void PrintError() {
  write(2, "Error in system call\n", strlen("Error in system call\n"));
}

ThreadPool *tpCreate(int numOfThreads) {
  if (numOfThreads < 1){
    return NULL;
  }
  ThreadPool *pool = (ThreadPool *) malloc(sizeof(ThreadPool));
  if (pool == NULL) {
    PrintError();
    exit(1);
  }
  pool->state = RUNNING;
  pool->numThreads = numOfThreads;
  pool->queue = osCreateQueue();
  if ((pthread_mutex_init(&pool->lock, NULL) != 0) || (pthread_cond_init(&pool->notify, NULL) != 0)) {
    PrintError();
    exit(1);
  }
  pool->threads = (pthread_t *) malloc(sizeof(pthread_t)*(size_t) numOfThreads);
  if (pool->threads == NULL) {
    PrintError();
    free(pool);
    exit(1);
  }
  int i;
  for (i = 0; i < numOfThreads; i++) {
    if (pthread_create(&(pool->threads[i]), NULL, ThreadFunction, (void *) pool) != 0) {
      tpDestroy(pool, 0);
      PrintError();
      exit(1);
    }
  }
  return pool;
}

int tpInsertTask(ThreadPool *pool, void (*computeFunc)(void *), void *param) {
  if (pool->state != RUNNING) {
    return 1;
  }
  Task *task = (Task *) calloc(sizeof(Task), 1);
  if (task == NULL) {
    PrintError();
    exit(1);
  }
  task->func = computeFunc;
  task->args = param;
  pthread_mutex_lock(&(pool->lock));

  osEnqueue(pool->queue, task);
  if (pthread_cond_broadcast(&(pool->notify)) != 0) {
    PrintError();
  }
  pthread_mutex_unlock(&(pool->lock));
  return 0;
}

static void *ThreadFunction(void *param) {
  ThreadPool *pool = (ThreadPool *) param;
  if (pool == NULL){
    PrintError();
    exit(1);
  }
  while (pool->state == RUNNING || (!osIsQueueEmpty(pool->queue) && pool->state == WAIT_STOP)) {
    pthread_mutex_lock(&pool->lock);
    while (pool->state == RUNNING && osIsQueueEmpty(pool->queue)) {
      pthread_cond_wait(&(pool->notify), &(pool->lock));
    }
    Task *task = (Task *) osDequeue(pool->queue);
    pthread_mutex_unlock(&pool->lock);
    if (task != NULL) {
      ((task->func))(task->args);
      free(task);
    }
  }
  pthread_mutex_unlock(&(pool->lock));
  pthread_exit(NULL);
}

void tpDestroy(ThreadPool *pool, int shouldWaitForTasks) {
  pthread_mutex_lock(&pool->lock);
  if (shouldWaitForTasks == 0) {
    pool->state = FORCE_STOP;
  }
  else {
    pool->state = WAIT_STOP;
  }
  pthread_cond_broadcast(&(pool->notify));
  pthread_mutex_unlock(&pool->lock);

  int i;
  for (i = 0; i < pool->numThreads; i++) {
    pthread_join(pool->threads[i], NULL);
  }
  while (!osIsQueueEmpty(pool->queue)) {
    Task * task = osDequeue(pool->queue);
    free(task);
  }
  free(pool->threads);
  osDestroyQueue(pool->queue);
  pthread_mutex_destroy(&pool->lock);
  pthread_cond_destroy(&pool->notify);
  free(pool);
}