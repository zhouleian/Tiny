#include "Queue.h"


int pq_init(pq_t* pq,size_t size,pq_comparator comp){
	pq->pq_ptr = (void**)malloc(sizeof(void*) * (size+1));
	if(!pq->pq_ptr){
		log_err("pq_init malloc failed");
		return -1;
	}
	pq->nalloc = 0;
	pq->size = size+1;
	pq->comp = comp;
	return OK;
}

int pq_is_empty(pq_t* pq){
	if(pq->nalloc == 0)
		return 1;
	return 0;
}
size_t pq_size(pq_t* pq){
	return pq->nalloc;
}

void* pq_min(pq_t* pq){
	if(pq->nalloc == 0)
		return NULL;
	return pq->pq_ptr[1];
}
static int resize(pq_t* pq,size_t new_size){
	if(new_size <= pq->nalloc){
		log_err("resize error: new size is too small");
		return -1;
	}
	void** new_ptr = (void**)malloc(sizeof(void*)*new_size);
	if(!new_ptr){
		log_err("resize error: malloc failed");
		return -1;
	}
	memcpy(new_ptr,pq->pq_ptr,sizeof(void*)*(pq->nalloc+1));
	free(pq->pq_ptr);
	pq->pq_ptr = new_ptr;
	pq->size = new_size;
	return OK;
}
//交换两个
static void exch(pq_t* pq,size_t i,size_t j){
	void* tmp = pq->pq_ptr[i];
	pq->pq_ptr[i] = pq->pq_ptr[j];
	pq->pq_ptr[j] = tmp;
	return;
}

static void swim(pq_t* pq,size_t k){
	while(k >1 && pq->comp(pq->pq_ptr[k],pq->pq_ptr[k/2])){
		exch(pq,k,k/2);
		k = k/2;
	}
}

static size_t sink(pq_t* pq,size_t k){
	int j;
	size_t nalloc = pq->nalloc;
	while(2*k < nalloc){
		j = 2*k;
		if(pq->comp(pq->pq_ptr[j+1],pq->pq_ptr[j]) && j < nalloc)
			j++;
		if(!pq->comp(pq->pq_ptr[k],pq->pq_ptr[k])) break;
		exch(pq,j,k);
		k = j;
	}
	return k;
}
//删除元素的时候，如果已分配元素数目< size/4，那么缩小到原来的1/2,即每次都要保留size是malloc的2倍
int pq_delmin(pq_t* pq){
	if(pq_is_empty(pq)){
		return OK;
	}
	size_t nalloc = pq->nalloc;
	exch(pq,1,nalloc);
	pq->nalloc--;
	sink(pq,1);
	if(pq->nalloc > 0 && nalloc <= (pq->size/4)){
		if(resize(pq,pq->size/2)<0)
			return -1;
	}
	return OK;
}

//插入
int pq_insert(pq_t* pq,void* item){
	if(pq->nalloc +1 == pq->size){
		//重分配空间
		if(resize(pq,pq->size*2) < 0)
			return -1;
	}
	pq->pq_ptr[++pq->nalloc] = item;
	swim(pq,pq->nalloc);
	return OK;
}

int pq_sink(pq_t* pq,size_t k){
	return sink(pq,k);
}





