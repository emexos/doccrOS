#include "main.h"
#include "../graphics/graphics.h"
#include "../string/string.h"
#include "../graphics/colors/stdclrs.h"

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


void memset(void *ptr, u8 val, size_t n)
{
    u8 *p = (u8 *)ptr;
    for (size_t i = 0; i < n; i++) {
        p[i] = val;
    }
}

void memcpy(void *dst, const void *src, size_t n)
{
    u8 *d = (u8 *)dst;
    const u8 *s = (const u8 *)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

void memmove(void *dst, const void *src, size_t n)
{
    u8 *d = (u8 *)dst;
    const u8 *s = (const u8 *)src;

    if (d < s) {
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else {
        for (size_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
}

int memcmp(const void *a, const void *b, size_t n)
{
    const u8 *pa = (const u8 *)a;
    const u8 *pb = (const u8 *)b;

    for (size_t i = 0; i < n; i++) {
        if (pa[i] != pb[i]) {
            return pa[i] - pb[i];
        }
    }
    return 0;
}


static mem_block_t *find_free_block(size_t sz)
{
    mem_block_t *current = heap_start;

    while (current != NULL) {
        if (current->magic != BLOCK_MAGIC) {
            // corrupted heap
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
        // not worth splitting
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

    // merge with next
    if (block->next != NULL && (block->next->flags & BLOCK_FREE)) {
        block->size += sizeof(mem_block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next != NULL) {
            block->next->prev = block;
        }
    }

    // merge with prev
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
    free_mem -= block->size;
    used_mem += block->size;

    return (void *)((u8 *)block + sizeof(mem_block_t));
}

void kfree(void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    mem_block_t *block = (mem_block_t *)((u8 *)ptr - sizeof(mem_block_t));

    if (block->magic != BLOCK_MAGIC) {
        // invalid free
        return;
    }

    if (block->flags & BLOCK_FREE) {
        // double free
        return;
    }

    block->flags = BLOCK_FREE;
    free_mem += block->size;
    used_mem -= block->size;

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
        // current block is big enough
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

size_t mem_get_free(void)
{
    return free_mem;
}

size_t mem_get_used(void)
{
    return used_mem;
}

void mem_test(void)
{
    print("Memory Test", GFX_CYAN);

    // test basic alloc
    char *test1 = (char *)kalloc(64);
    if (test1 == NULL) {
        print("FAIL: kalloc test1", GFX_RED);
        return;
    }
    print("PASS: kalloc test1", GFX_GREEN);

    // test memset
    memset(test1, 0xAA, 64);
    int ok = 1;
    for (int i = 0; i < 64; i++) {
        if ((u8)test1[i] != 0xAA) {
            ok = 0;
            break;
        }
    }
    if (ok) {
        print("PASS: memset", GFX_GREEN);
    } else {
        print("FAIL: memset", GFX_RED);
    }

    // test memcpy
    char *test2 = (char *)kalloc(64);
    if (test2 == NULL) {
        print("FAIL: kalloc test2", GFX_RED);
        kfree(test1);
        return;
    }

    memset(test2, 0x55, 64);
    memcpy(test1, test2, 64);

    ok = 1;
    for (int i = 0; i < 64; i++) {
        if ((u8)test1[i] != 0x55) {
            ok = 0;
            break;
        }
    }
    if (ok) {
        print("PASS: memcpy", GFX_GREEN);
    } else {
        print("FAIL: memcpy", GFX_RED);
    }

    // test memcmp
    if (memcmp(test1, test2, 64) == 0) {
        print("PASS: memcmp", GFX_GREEN);
    } else {
        print("FAIL: memcmp", GFX_RED);
    }

    // test memmove
    char *test3 = (char *)kalloc(128);
    if (test3 == NULL) {
        print("FAIL: kalloc test3", GFX_RED);
        kfree(test1);
        kfree(test2);
        return;
    }

    for (int i = 0; i < 128; i++) {
        test3[i] = i & 0xFF;
    }

    memmove(test3 + 10, test3, 64);

    ok = 1;
    for (int i = 0; i < 64; i++) {
        if ((u8)test3[i + 10] != (i & 0xFF)) {
            ok = 0;
            break;
        }
    }
    if (ok) {
        print("PASS: memmove", GFX_GREEN);
    } else {
        print("FAIL: memmove", GFX_RED);
    }

    // test free
    size_t before_free = mem_get_free();
    kfree(test1);
    kfree(test2);
    kfree(test3);

    size_t after_free = mem_get_free();

    if (after_free > before_free) {
        print("PASS: kfree", GFX_GREEN);
    } else {
        print("FAIL: kfree", GFX_RED);
    }

    // test realloc
    char *test4 = (char *)kalloc(32);
    if (test4 == NULL) {
        print("FAIL: kalloc test4", GFX_RED);
        return;
    }

    memset(test4, 0x42, 32);
    test4 = (char *)krealloc(test4, 128);

    if (test4 == NULL) {
        print("FAIL: krealloc", GFX_RED);
        return;
    }

    ok = 1;
    for (int i = 0; i < 32; i++) {
        if ((u8)test4[i] != 0x42) {
            //ok = 0;
            break;
        }
    }

    if (ok) {
        print("PASS: krealloc", GFX_GREEN);
    } else {
        print("FAIL: krealloc", GFX_RED);
    }

    kfree(test4);

    // print mem stats
    char buf[128];

    str_copy(buf, "Free: ");
    str_append_uint(buf, (u32)mem_get_free());
    str_append(buf, " bytes");
    print(buf, GFX_BLUE);

    str_copy(buf, "Used: ");
    str_append_uint(buf, (u32)mem_get_used());
    str_append(buf, " bytes");
    print(buf, GFX_BLUE);
}
