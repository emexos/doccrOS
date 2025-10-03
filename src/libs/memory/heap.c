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

extern mem_block_t* mem_get_heap_start(void);
extern void mem_update_stats(size_t free_delta, size_t used_delta, int add);

static mem_block_t *find_free_block(size_t sz)
{
    mem_block_t *current = mem_get_heap_start();

    while (current != NULL) {
        if (current->magic != BLOCK_MAGIC) {
            // heap corruption detected
            return NULL;
        }

        if ((current->flags & BLOCK_FREE) && current->size >= sz) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

static void split_block(mem_block_t *block, size_t sz)
{
    if (block->size < sz + sizeof(mem_block_t) + 16) {
        // block too small to split
        return;
    }

    mem_block_t *new_block = (mem_block_t *)((u8 *)block + sizeof(mem_block_t) + sz);
    new_block->magic = BLOCK_MAGIC;
    new_block->size = block->size - sz - sizeof(mem_block_t);
    new_block->flags = BLOCK_FREE;
    new_block->next = block->next;
    new_block->prev = block;

    if (block->next != NULL) {
        block->next->prev = new_block;
    }

    block->size = sz;
    block->next = new_block;
}

static void merge_free_blocks(mem_block_t *block)
{
    if (block == NULL || !(block->flags & BLOCK_FREE)) {
        return;
    }

    // merge with next block
    if (block->next != NULL && (block->next->flags & BLOCK_FREE)) {
        block->size += sizeof(mem_block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next != NULL) {
            block->next->prev = block;
        }
    }

    // merge with previous block
    if (block->prev != NULL && (block->prev->flags & BLOCK_FREE)) {
        block->prev->size += sizeof(mem_block_t) + block->size;
        block->prev->next = block->next;
        if (block->next != NULL) {
            block->next->prev = block->prev;
        }
    }
}

void *kalloc(size_t sz)
{
    if (sz == 0) {
        return NULL;
    }

    // align to 8 bytes
    if (sz % 8 != 0) {
        sz = (sz / 8 + 1) * 8;
    }

    mem_block_t *block = find_free_block(sz);
    if (block == NULL) {
        return NULL;
    }

    split_block(block, sz);

    block->flags = BLOCK_USED;
    mem_update_stats(block->size, block->size, 0);

    return (void *)((u8 *)block + sizeof(mem_block_t));
}

void kfree(void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    mem_block_t *block = (mem_block_t *)((u8 *)ptr - sizeof(mem_block_t));

    if (block->magic != BLOCK_MAGIC) {
        // invalid pointer
        return;
    }

    if (block->flags & BLOCK_FREE) {
        // double free detected
        return;
    }

    block->flags = BLOCK_FREE;
    mem_update_stats(block->size, block->size, 1);

    merge_free_blocks(block);
}

void *krealloc(void *ptr, size_t new_sz)
{
    if (ptr == NULL) {
        return kalloc(new_sz);
    }

    if (new_sz == 0) {
        kfree(ptr);
        return NULL;
    }

    mem_block_t *block = (mem_block_t *)((u8 *)ptr - sizeof(mem_block_t));

    if (block->magic != BLOCK_MAGIC) {
        return NULL;
    }

    if (block->size >= new_sz) {
        // current block is sufficient
        return ptr;
    }

    void *new_ptr = kalloc(new_sz);
    if (new_ptr == NULL) {
        return NULL;
    }

    memcpy(new_ptr, ptr, block->size);
    kfree(ptr);

    return new_ptr;
}
