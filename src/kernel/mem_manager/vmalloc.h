#ifndef VMALLOC_H
#define VMALLOC_H

#include "../../../shared/types.h"
#include "virtmem.h"

// Virtuelle Speicherallokation für Kernel und Userspace
#define KERNEL_HEAP_START   0xFFFFFFFF80000000ULL
#define KERNEL_HEAP_SIZE    (64 * 1024 * 1024)  // 64 MB

#define USER_HEAP_START     0x0000000040000000ULL
#define USER_HEAP_SIZE      (128 * 1024 * 1024) // 128 MB

typedef struct vmalloc_region {
    u64 virt_start;
    u64 size;
    u64 flags;
    struct vmalloc_region* next;
} vmalloc_region_t;

// Initialisierung
void vmalloc_init(void);

// Kernel virtuelle Allokation
void* kvmalloc(size_t size);
void kvfree(void* ptr);

// User virtuelle Allokation (für Prozesse)
void* uvmalloc(pml4_t* pml4, size_t size);
void uvfree(pml4_t* pml4, void* ptr);

// Utility
void vmalloc_dump_regions(void);

#endif
