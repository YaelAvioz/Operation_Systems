//Yael Avioz 207237421

#include "threadPool.h"

ThreadPool *tpCreate(int numOfThreads) {
  //input validation
  if (numOfThreads < 1){
    return NULL;
  }

  //create new thread pool
  ThreadPool *threadPool = (ThreadPool *) malloc(sizeof(ThreadPool));

  //chcek if the allocation succeeded
  if (threadPool == NULL) {
    exit(1);
  }

  threadPool->state = RUNNING;
  threadPool->threadsNumber = numOfThreads;
  threadPool->queue = osCreateQueue();

  //in case threadpool->lock and threadpool->notify are not null exit
  if ((pthread_mutex_init(&threadPool->lock, NULL) != 0) || (pthread_cond_init(&threadPool->notify, NULL) != 0)) {
    exit(1);
  }

  //allocate memory to threadpool->threads
  threadPool->threads = (pthread_t *) malloc(sizeof(pthread_t)*(size_t) numOfThreads);

  //chcek if the allocation succeeded
  if (threadPool->threads == NULL) {
    free(threadPool);
    exit(1);
  }

  //starts a new threads in the calling process
  int i;
  for (i = 0; i < numOfThreads; i++) {
    if (pthread_create(&(threadPool->threads[i]), NULL, startRoutine, (void *)threadPool) != 0) {
      tpDestroy(threadPool, 0);
      exit(1);
    }
  }
  return threadPool;
}

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {

  //lock the threadPool
  pthread_mutex_lock(&threadPool->lock);
  if (shouldWaitForTasks == 0) {
	  threadPool->state = STOP;
  }
  else {
	  threadPool->state = WAIT;
  }

  //unblock all threads currently blocked on the specified condition variable cond
  pthread_cond_broadcast(&(threadPool->notify));
  pthread_mutex_unlock(&threadPool->lock);


  //this loop waits for the thread specified by thread to terminate
  int i;
  for (i = 0; i < threadPool->threadsNumber; i++) {
    pthread_join(threadPool->threads[i], NULL);
  }

  //free task
  while (!osIsQueueEmpty(threadPool->queue)) {
    Task * task = osDequeue(threadPool->queue);
    free(task);
  }

  //destroy
  free(threadPool->threads);
  osDestroyQueue(threadPool->queue);
  pthread_mutex_destroy(&threadPool->lock);
  pthread_cond_destroy(&threadPool->notify);
  free(threadPool);
}

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc)(void*), void* param) {

	//in case is running return 1
	if (threadPool->state != RUNNING) {
		return 1;
	}

	//allocates the requested memory and returns a pointer to it
	Task* task = (Task*)calloc(sizeof(Task), 1);

	//in case of faliure
	if (task == NULL) {
		exit(1);
	}
	task->func = computeFunc;
	task->args = param;

	//unlock threadpool
	pthread_mutex_lock(&(threadPool->lock));

	osEnqueue(threadPool->queue, task);

	//unlock threadpool
	pthread_mutex_unlock(&(threadPool->lock));
	return 0;
}

//This is the start routine function
static void* startRoutine(void* param) {

	//set threadpool
	ThreadPool* threadPool = (ThreadPool*)param;
	//in case of faliure
	if (threadPool == NULL) {
		exit(1);
	}
	while (threadPool->state == RUNNING || (!osIsQueueEmpty(threadPool->queue) && threadPool->state == WAIT)) {
		pthread_mutex_lock(&threadPool->lock);
		while (threadPool->state == RUNNING && osIsQueueEmpty(threadPool->queue)) {
			pthread_cond_wait(&(threadPool->notify), &(threadPool->lock));
		}
		Task* task = (Task*)osDequeue(threadPool->queue);
		pthread_mutex_unlock(&threadPool->lock);
		if (task != NULL) {
			((task->func))(task->args);
			free(task);
		}
	}
	//unlock
	pthread_mutex_unlock(&(threadPool->lock));
	pthread_exit(NULL);
}
