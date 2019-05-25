#include "malloc.h"
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/mman.h>
extern void* sbrk(intptr_t);
static pthread_mutex_t lk=PTHREAD_MUTEX_INITIALIZER;
//=========================================================
// Your implementations HERE
/*
typedef struct free_block {
    size_t size;
    struct free_block* next;
} free_block;
static free_block free_block_list_head = { 0, NULL };
static const size_t align_to = 16;

static void* _malloc(size_t size) 
{
    if(!size)
        return NULL;
    size = (size + sizeof(free_block) + (align_to - 1)) & ~ (align_to - 1);
    free_block* block = free_block_list_head.next;
    free_block** head = &(free_block_list_head.next);
    while (block != NULL) 
    {
        if (block->size >= size) 
        {
            *head = block->next;
            return ((char*)block) + sizeof(free_block);                 
        }
        head = &(block->next);
        block = block->next;                                   
    }
    block = (free_block*)sbrk(size);
    block->size = size;
    return ((char*)block) + sizeof(free_block);
}
static void _free(void* ptr) {
    if(!ptr)
        return;
    free_block* block = (free_block*)(((char*)ptr) - sizeof(free_block));
    block->next = free_block_list_head.next;
    free_block_list_head.next = block;
}
//=========================================================
*/
void* _malloc(size_t len) {
    void* addr = mmap(0,
                        len + sizeof(size_t),
                        PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE,
                        -1,                                                                         0);
    *(size_t*)addr = len;
    return addr + sizeof(size_t);
}

int _free(void* addr) {
      return munmap(addr - sizeof(size_t), (size_t) addr);

}
void* do_malloc(size_t size) {
    pthread_mutex_lock(&lk);
    void* ret = _malloc(size);
    pthread_mutex_unlock(&lk);
    return ret;
}

void do_free(void *ptr) {
    pthread_mutex_lock(&lk);
    _free(ptr);
    pthread_mutex_unlock(&lk);
}
