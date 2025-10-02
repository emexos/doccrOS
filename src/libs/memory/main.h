#ifndef MEM_H
#define MEM_H

#include "../../../shared/types.h"

// mem ops
void memset(void *ptr, u8 val, size_t n);
void memcpy(void *dst, const void *src, size_t n);
void memmove(void *dst, const void *src, size_t n);
int memcmp(const void *a, const void *b, size_t n);

// heap
void mem_init(void *heap_start, size_t heap_sz);
void *kalloc(size_t sz);
void kfree(void *ptr);
void *krealloc(void *ptr, size_t new_sz);

// debug/test
void mem_test(void);
size_t mem_get_free(void);
size_t mem_get_used(void);

#endif
