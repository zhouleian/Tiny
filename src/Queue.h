#ifndef SHMQUEUE_H
#define SHMQUEUE_H
#include "Log.h"
#include "DefineConf.h"

#define QUEUE_SIZE 10
typedef int(*pq_comparator)(void* p,void* q);

typedef struct{
	void** pq_ptr;
	size_t nalloc;//实际大小
	size_t size;//初始化大小，容纳能力
	pq_comparator comp;
}pq_t;

int pq_init(pq_t* pq,size_t size,pq_comparator comp);
int pq_is_empty(pq_t* pq);

size_t pq_size(pq_t* pq);
void* pq_min(pq_t* pq);

int pq_delmin(pq_t* pq);
int pq_insert(pq_t* pq,void* item);
int pq_sink(pq_t* pq,size_t k);


#endif
