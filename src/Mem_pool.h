#ifndef MEM_POOL_H
#define MEM_POOL_H
#include <stdlib.h>
#define tiny_free free
typedef unsigned char u_char;
typedef unsigned int tiny_uint_t;
/*
last：是一个unsigned char 类型的指针，保存的是/当前内存池分配到末位地址，即下一次分配从此处开始。
end：内存池结束位置；
next：内存池里面有很多块内存，这些内存块就是通过该指针连成链表的，next指向下一块内存。
failed：内存池分配失败次数。

tiny_pool_s
d：内存池的数据块；
max：内存池数据块的最大值；
current：指向当前内存池；
chain：该指针挂接一个tiny_chain_t结构；
large：大块内存链表，即分配空间超过max的情况使用；
cleanup：释放内存池的callback
log：日志信息
*/
typedef struct tiny_pool_s tiny_pool_t;
typedef struct {
	u_char *last;
	u_char *end;
	tiny_pool_t *next;
	tiny_uint_t failed;
}tiny_pool_data_t;
//大块数据,当一个申请的内存空间大小比内存池的大小还要大的时候，malloc一块大的空间，再内存池用保留这个地址的指针。
typedef struct tiny_pool_large_s tiny_pool_large_t; 
struct tiny_pool_large_s {
	tiny_pool_large_t* next;
    void* alloc;
};
/*
typedef void (*tiny_pool_cleanup_pt)(void *data);
typedef struct tiny_pool_cleanup_s tiny_pool_cleanup_t;  
struct tiny_pool_cleanup_s {
    tiny_pool_cleanup_pt handler;
    void* data;
    tiny_pool_cleanup_t* next;
};
*/
struct tiny_pool_s{
	tiny_pool_data_t d;
	size_t max;
	tiny_pool_t *current;
	tiny_pool_large_t     *large;
};


tiny_pool_t* tiny_create_pool(size_t size);
void* tiny_palloc(tiny_pool_t* pool, size_t size);
void tiny_destroy_pool(tiny_pool_t* pool);
static void* tiny_palloc_block(tiny_pool_t* pool, size_t size);




#endif
