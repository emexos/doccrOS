#ifndef VIRTMEM_H
#define VIRTMEM_H

#include "../../../shared/types.h"
#include "physmem.h"

// Virtueller Speicher - Paging
#define PML4_ENTRIES 512
#define PDPT_ENTRIES 512
#define PD_ENTRIES 512
#define PT_ENTRIES 512

#define PAGE_PRESENT    (1ULL << 0)
#define PAGE_WRITE      (1ULL << 1)
#define PAGE_USER       (1ULL << 2)
#define PAGE_ACCESSED   (1ULL << 5)
#define PAGE_DIRTY      (1ULL << 6)

typedef u64 pml4_entry_t;
typedef u64 pdpt_entry_t;
typedef u64 pd_entry_t;
typedef u64 pt_entry_t;

typedef struct {
    pml4_entry_t entries[PML4_ENTRIES];
} __attribute__((aligned(PAGE_SIZE))) pml4_t;

typedef struct {
    pdpt_entry_t entries[PDPT_ENTRIES];
} __attribute__((aligned(PAGE_SIZE))) pdpt_t;

typedef struct {
    pd_entry_t entries[PD_ENTRIES];
} __attribute__((aligned(PAGE_SIZE))) pd_t;

typedef struct {
    pt_entry_t entries[PT_ENTRIES];
} __attribute__((aligned(PAGE_SIZE))) pt_t;

// Virtuelle Speicher-Funktionen
void virtmem_init(void);
pml4_t* virtmem_create_address_space(void);
void virtmem_destroy_address_space(pml4_t* pml4);
int virtmem_map_page(pml4_t* pml4, u64 virt_addr, u64 phys_addr, u64 flags);
void virtmem_unmap_page(pml4_t* pml4, u64 virt_addr);
u64 virtmem_get_physical(pml4_t* pml4, u64 virt_addr);
void virtmem_switch_context(pml4_t* pml4);

#endif
