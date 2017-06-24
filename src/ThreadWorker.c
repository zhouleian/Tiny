#include "ThreadWorker.h"
#include "Log.h"

void* thread_worker(void *arg){
	if(arg == NULL){
		log_err("arg should be type thread_pool*");
		return NULL;
	}
	thread_pool* pool = (thread_pool*)arg;
	t_task * task;
	while(true){
		pthread_mutex_lock(&(pool->mutex_));
		while((pool->queue_size == 0) && pool->is_open)//还没有线程时
			pthread_cond_wait(&(pool->cond_),&(pool->mutex_));
		if(!pool->is_open)
			break;
		task = pool->head->next;//取到任务
		if(task == NULL){//还没有任务
			pthread_mutex_unlock(&(pool->mutex_));
			continue;
		}
		pool->head->next = task->next;
		pool->queue_size--;
		pthread_mutex_unlock(&(pool->lock));
		//取到任务，执行
		(*(task->func))(task->arg);
		free(task);
	}
	pool->started--;
	pthread_mutex_unlock(&(pool->lock));
	pthread_exit(NULL);
	return NULL;
}



