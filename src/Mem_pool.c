#include "Mem_pool.h"
tiny_pool_t* tiny_create_pool(size_t size){
	tiny_pool_t*		p;

	p = (tiny_pool_t*)malloc(size);
	if (!p){
		return NULL;
	}

	//计算内存池的数据区域
	p->d.last = (u_char*)p + sizeof(tiny_pool_t);
	//printf("%d\n",sizeof(tiny_pool_t));//十六进制40,十进制64
	//printf("%d\n",size);

	p->d.end = (u_char*)p + size;
	p->d.next = NULL;//下个内存池
	p->d.failed = 0;

	size = size - sizeof(tiny_pool_t);
	p->max = size;//最大数据

	//我现在还是是一个单一的内存池
	p->current = p;
	//p->chain = NULL;
	//只有在需要的时候才分配大的内存区域
	p->large = NULL;
	p->cleanup = NULL;

	return p;
}
//一块新的内存块，为poll内存池开辟一个新的内存块，申请size大小的内存
static void* tiny_palloc_block(tiny_pool_t* pool, size_t size){
	printf("tiny_palloc_block-------------\n");
	u_char      *m;
	size_t       psize;
	tiny_pool_t  *p, *pnew, *current;

	psize = (size_t) (pool->d.end - (u_char *) pool);
	//printf("%d----\n",psize);
	m = (u_char*)malloc(psize);
	if (!m){
		return	NULL;
	}

	//一个新的内存池
	pnew = (tiny_pool_t*) m;
	pnew->d.end = m +psize;
	pnew->d.next = NULL;
	pnew->d.failed = 0;
//	这儿有个细节，新的节点可以用ngx_pool_t指针表示，但具体的数据存储则是ngx_pool_data_t.
	//是不是和tiny_palloc很相似啊
	///让m指向该块内存tiny_pool_data_t结构体之后数据区起始位
	m += sizeof(tiny_pool_data_t);
	//在数据区分配size大小的内存并设置last指针
	
	pnew->d.last = m + size;

	current = pool->current;
	//遍历到内存池链表的末尾
	//这里的循环用来找最后一个链表节点，这里failed用来控制循环的长度，如果分配失败次数达到5次，就忽略，不需要每次都从头找起,改变了current
	for (p = current; p->d.next; p = p->d.next) {
		printf("***************\n");
		if (p->d.failed++ > 4) {//为什么4？推测是个经验值
			current = p->d.next;
		}
	}

	p->d.next = pnew;

	pool->current = current ? current : pnew;

	return m;
}

static void *tiny_palloc_large(tiny_pool_t *pool, size_t size){
	printf("tiny_palloc_large-------------\n");
	void              *p;
	tiny_uint_t         n;
	tiny_pool_large_t  *large;
	// 直接在系统堆中分配一块空间  
	p = (tiny_pool_t*)malloc(size);
	if (p == NULL) {
		return NULL;
	}

	n = 0;
	// 查找到一个空的large区，如果有，则将刚才分配的空间交由它管理  
	for (large = pool->large; large; large = large->next) {
		if (large->alloc == NULL) {
			large->alloc = p;
			return p;
		}

		if (n++ > 3) {
			break;
		}
	}
	//为了提高效率， 如果在三次内没有找到空的large结构体，则创建一个
	large = tiny_palloc(pool, sizeof(tiny_pool_large_t));
	if (large == NULL) {
		tiny_free(p);
		return NULL;
	}

	large->alloc = p;
	large->next = pool->large;
	pool->large = large;

	return p;
}



void* tiny_palloc(tiny_pool_t* pool, size_t size){
	u_char*			m;
	tiny_pool_t*		p;

	//遍历内存池，拿出可用的内存区域
	if (size <= pool->max){
		p = pool->current;

		do {
			m = p->d.last;

			if ((size_t)(p->d.end - m) >= size) {
				p->d.last = m + size;//用掉了当然要改变*last了
				return m;
			}

			p = p->d.next;
		} while (p);
		return tiny_palloc_block(pool, size);
		//所有的内存池都已经满了，即链表里没有能分配size大小内存的节点，则生成一个新的节点并在其中分配内存  
	}
	//申请的内存超过了内存池的大小，所以用
	return tiny_palloc_large(pool, size);
}


void tiny_destroy_pool(tiny_pool_t* pool)
{
	tiny_pool_t          *p, *n;
	tiny_pool_large_t    *l;
	tiny_pool_cleanup_t  *c;

	//调用清理函数
	for (c = pool->cleanup; c; c = c->next) {
		printf("cleanup -----------------\n");
		if (c->handler) {		
			c->handler(c->data);
		}
	}

	//释放大块的内存
	printf("des large -----------------\n");
	for (l = pool->large; l; l = l->next) {
		if (l->alloc) {
			free(l->alloc);
		}
	}

	//小块的内存，真正意义上的内存池
	printf("des pool -----------------\n");
	for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
		free(p);

		//如果当前内存池为空，之后的毕为空
		if (n == NULL) {
			break;
		}
	}
}


