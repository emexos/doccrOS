#include "main.h"

#define BLOCK_MAGIC 0xDEADBEEF
#define BLOCK_FREE 0x1
#define BLOCK_USED 0x2

typedef struct mem_block {
    u32 magic;
    size_t size;
    u32 flags;
    struct mem_block *next;
    struct mem_block *prev;
} mem_block_t;

static mem_block_t *heap_start = NULL;
static size_t total_heap = 0;
static size_t free_mem = 0;
static size_t used_mem = 0;

void mem_init(void *heap_addr, size_t heap_sz)
{
    heap_start = (mem_block_t *)heap_addr;
    total_heap = heap_sz;

    heap_start->magic = BLOCK_MAGIC;
    heap_start->size = heap_sz - sizeof(mem_block_t);
    heap_start->flags = BLOCK_FREE;
    heap_start->next = NULL;
    heap_start->prev = NULL;

    free_mem = heap_start->size;
    used_mem = 0;
}

size_t mem_get_free(void)
{
    return free_mem;
}

size_t mem_get_used(void)
{
    return used_mem;
}

mem_block_t* mem_get_heap_start(void)
{
    return heap_start;
}

void mem_update_stats(size_t free_delta, size_t used_delta, int add)
{
    if (add) {
        free_mem += free_delta;
        used_mem += used_delta;
    } else {
        free_mem -= free_delta;
        used_mem -= used_delta;
    }
}
