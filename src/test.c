#include "Mem_pool.h"
#include <stdio.h>
void dump_pool(tiny_pool_t* pool)  {  
	while (pool){  
		printf("pool = 0x%x\n", pool);  
		printf("  .d\n");  
		printf("    .last = 0x%x\n", pool->d.last);  
		printf("    .end = 0x%x\n", pool->d.end);  
		printf("    .next = 0x%x\n", pool->d.next);  
		printf("    .failed = %d\n", pool->d.failed);  
		printf("  .max = %d\n", pool->max);  
		printf("  .current = 0x%x\n", pool->current);  
		//printf("  .chain = 0x%x\n", pool->chain);  
		printf("  .large = 0x%x\n", pool->large);  
		//printf("  .cleanup = 0x%x\n", pool->cleanup);  
		//printf("  .log = 0x%x\n", pool->log);  
		printf("available pool memory = %d\n\n", pool->d.end - pool->d.last);  
		pool = pool->d.next;  
	}  
} 
int main()  {  
    tiny_pool_t *pool;  

  
    printf("--------------------------------\n");  
    printf("create a new pool:\n");  
    printf("--------------------------------\n");  
    pool = tiny_create_pool(1024);  
    dump_pool(pool);  

    printf("--------------------------------\n");  
    printf("alloc block 1 from the pool:\n");  
    printf("--------------------------------\n");  
    tiny_palloc(pool, 512);  
    dump_pool(pool);  
  
    printf("--------------------------------\n");  
    printf("alloc block 2 from the pool:\n");  
    printf("--------------------------------\n");  
    tiny_palloc(pool, 512);  
    dump_pool(pool);  

	 printf("--------------------------------\n");  
    printf("alloc block 3 from the pool:\n");  
    printf("--------------------------------\n");  
    tiny_palloc(pool, 512);  
    dump_pool(pool);  

	 printf("--------------------------------\n");  
    printf("alloc block 4 from the pool:\n");  
    printf("--------------------------------\n");  
    tiny_palloc(pool, 512);  
    dump_pool(pool);  


    printf("--------------------------------\n");  
    printf("alloc block 5 from the pool :\n");  
    printf("--------------------------------\n");  
    tiny_palloc(pool, 512);  
    dump_pool(pool);  

	 printf("--------------------------------\n");  
    printf("alloc block 6 from the pool:\n");  
    printf("--------------------------------\n");  
    tiny_palloc(pool, 512);  
    dump_pool(pool);  
	printf("--------------------------------\n");  
    printf("alloc block 7 from the pool:\n");  
    printf("--------------------------------\n");  
    tiny_palloc(pool, 512);  
    dump_pool(pool);  
	printf("--------------------------------\n");  
    printf("alloc block 8 from the pool:\n");  
    printf("--------------------------------\n");  
    tiny_palloc(pool, 512);  
    dump_pool(pool);  


	 printf("--------------------------------\n");  
    printf("alloc block 9 from the pool:\n");  
    printf("--------------------------------\n");  
    tiny_palloc(pool,1024);  
    dump_pool(pool); 

	/*

		 printf("--------------------------------\n");  
    printf("alloc block 7 from the pool:\n");  
    printf("--------------------------------\n");  
    tiny_palloc(pool,1024);  
    dump_pool(pool); 
		 printf("--------------------------------\n");  
    printf("alloc block 7 from the pool:\n");  
    printf("--------------------------------\n");  
    tiny_palloc(pool,1024);  
    dump_pool(pool); 
		 printf("--------------------------------\n");  
    printf("alloc block 7 from the pool:\n");  
    printf("--------------------------------\n");  
    tiny_palloc(pool,1024);  
    dump_pool(pool); 
*/



    tiny_destroy_pool(pool);  
    return 0;  
}  
