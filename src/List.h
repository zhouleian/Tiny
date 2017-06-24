#ifndef LIST_H
#define LIST_H
//使用双向链表
struct list_head {
    struct list_head *prev, *next;
};

typedef struct list_head list_head;
//初始化链表头节点，循环
static void Init_list_head(struct list_head *ptr){
    struct list_head *_ptr = (struct list_head *)ptr;
    (_ptr)->next = (_ptr); 
	(_ptr->prev) = (_ptr);
}

//添加node节点
static inline void add_node(struct list_head *_new, struct list_head *prev, struct list_head *next) {
    _new->next = next;
    next->prev = _new;
    prev->next = _new;
    _new->prev = prev;
}

static inline void list_add(struct list_head *_new, struct list_head *head) {
    add_node(_new, head, head->next);
}

static inline void add_node_tail(struct list_head *_new, struct list_head *head) {
    add_node(_new, head->prev, head);
}


static inline void del_node(struct list_head *prev, struct list_head *next) {
    prev->next = next;
    next->prev = prev;
}

static inline void list_del(struct list_head *entry) {
    del_node(entry->prev, entry->next);
}


static inline int list_empty(struct list_head *head) {
    return (head->next == head) && (head->prev == head);
}

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
/*
#define list_entry(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );     \
*/
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );     \
})

#define list_entry(ptr, type, member) container_of(ptr, type, member)
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)



#endif 
