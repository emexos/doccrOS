#ifndef PHYSMEM_H
#define PHYSMEM_H

#include "../../../shared/types.h"

#define PAGE_SIZE 4096
#define PHYS_MEM_MAX (512 * 1024 * 1024) // 512 MB max
#define BITMAP_SIZE (PHYS_MEM_MAX / PAGE_SIZE / 8)

// Physische Speicherverwaltung
void physmem_init(u64 mem_size);
u64 physmem_alloc_page(void);
void physmem_free_page(u64 phys_addr);
u64 physmem_get_free_pages(void);
u64 physmem_get_used_pages(void);

// Bitmap für freie Seiten
typedef struct {
    u8 bitmap[BITMAP_SIZE];
    u64 total_pages;
    u64 free_pages;
    u64 used_pages;
} physmem_manager_t;

#endif
