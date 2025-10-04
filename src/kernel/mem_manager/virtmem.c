#include "virtmem.h"
#include "physmem.h"
#include "../../libs/memory/main.h"

static pml4_t* kernel_pml4 = NULL;

static inline void load_cr3(u64 pml4_phys)
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(pml4_phys) : "memory");
}

static inline u64 get_cr3(void)
{
    u64 cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

void virtmem_init(void)
{
    // Kernel PML4 wird später initialisiert
    kernel_pml4 = NULL;
}

pml4_t* virtmem_create_address_space(void)
{
    u64 pml4_phys = physmem_alloc_page();
    if (pml4_phys == 0) {
        return NULL;
    }

    pml4_t* pml4 = (pml4_t*)pml4_phys;
    memset(pml4, 0, PAGE_SIZE);

    return pml4;
}

void virtmem_destroy_address_space(pml4_t* pml4)
{
    if (pml4 == NULL || pml4 == kernel_pml4) {
        return;
    }

    // Durchlaufe alle Einträge und gebe Seiten frei
    for (int pml4_idx = 0; pml4_idx < PML4_ENTRIES; pml4_idx++) {
        if (!(pml4->entries[pml4_idx] & PAGE_PRESENT)) {
            continue;
        }

        pdpt_t* pdpt = (pdpt_t*)(pml4->entries[pml4_idx] & ~0xFFF);

        for (int pdpt_idx = 0; pdpt_idx < PDPT_ENTRIES; pdpt_idx++) {
            if (!(pdpt->entries[pdpt_idx] & PAGE_PRESENT)) {
                continue;
            }

            pd_t* pd = (pd_t*)(pdpt->entries[pdpt_idx] & ~0xFFF);

            for (int pd_idx = 0; pd_idx < PD_ENTRIES; pd_idx++) {
                if (!(pd->entries[pd_idx] & PAGE_PRESENT)) {
                    continue;
                }

                pt_t* pt = (pt_t*)(pd->entries[pd_idx] & ~0xFFF);

                // Gebe alle Page Table Seiten frei
                for (int pt_idx = 0; pt_idx < PT_ENTRIES; pt_idx++) {
                    if (pt->entries[pt_idx] & PAGE_PRESENT) {
                        u64 phys = pt->entries[pt_idx] & ~0xFFF;
                        physmem_free_page(phys);
                    }
                }

                physmem_free_page((u64)pt);
            }

            physmem_free_page((u64)pd);
        }

        physmem_free_page((u64)pdpt);
    }

    physmem_free_page((u64)pml4);
}

int virtmem_map_page(pml4_t* pml4, u64 virt_addr, u64 phys_addr, u64 flags)
{
    if (pml4 == NULL) {
        return -1;
    }

    // Extrahiere Indizes aus virtueller Adresse
    u64 pml4_idx = (virt_addr >> 39) & 0x1FF;
    u64 pdpt_idx = (virt_addr >> 30) & 0x1FF;
    u64 pd_idx = (virt_addr >> 21) & 0x1FF;
    u64 pt_idx = (virt_addr >> 12) & 0x1FF;

    // PDPT holen oder erstellen
    pdpt_t* pdpt;
    if (!(pml4->entries[pml4_idx] & PAGE_PRESENT)) {
        u64 pdpt_phys = physmem_alloc_page();
        if (pdpt_phys == 0) return -1;

        pdpt = (pdpt_t*)pdpt_phys;
        memset(pdpt, 0, PAGE_SIZE);
        pml4->entries[pml4_idx] = pdpt_phys | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    } else {
        pdpt = (pdpt_t*)(pml4->entries[pml4_idx] & ~0xFFF);
    }

    // PD holen oder erstellen
    pd_t* pd;
    if (!(pdpt->entries[pdpt_idx] & PAGE_PRESENT)) {
        u64 pd_phys = physmem_alloc_page();
        if (pd_phys == 0) return -1;

        pd = (pd_t*)pd_phys;
        memset(pd, 0, PAGE_SIZE);
        pdpt->entries[pdpt_idx] = pd_phys | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    } else {
        pd = (pd_t*)(pdpt->entries[pdpt_idx] & ~0xFFF);
    }

    // PT holen oder erstellen
    pt_t* pt;
    if (!(pd->entries[pd_idx] & PAGE_PRESENT)) {
        u64 pt_phys = physmem_alloc_page();
        if (pt_phys == 0) return -1;

        pt = (pt_t*)pt_phys;
        memset(pt, 0, PAGE_SIZE);
        pd->entries[pd_idx] = pt_phys | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    } else {
        pt = (pt_t*)(pd->entries[pd_idx] & ~0xFFF);
    }

    // Mappe Seite
    pt->entries[pt_idx] = (phys_addr & ~0xFFF) | flags | PAGE_PRESENT;

    return 0;
}

void virtmem_unmap_page(pml4_t* pml4, u64 virt_addr)
{
    if (pml4 == NULL) {
        return;
    }

    u64 pml4_idx = (virt_addr >> 39) & 0x1FF;
    u64 pdpt_idx = (virt_addr >> 30) & 0x1FF;
    u64 pd_idx = (virt_addr >> 21) & 0x1FF;
    u64 pt_idx = (virt_addr >> 12) & 0x1FF;

    if (!(pml4->entries[pml4_idx] & PAGE_PRESENT)) return;
    pdpt_t* pdpt = (pdpt_t*)(pml4->entries[pml4_idx] & ~0xFFF);

    if (!(pdpt->entries[pdpt_idx] & PAGE_PRESENT)) return;
    pd_t* pd = (pd_t*)(pdpt->entries[pdpt_idx] & ~0xFFF);

    if (!(pd->entries[pd_idx] & PAGE_PRESENT)) return;
    pt_t* pt = (pt_t*)(pd->entries[pd_idx] & ~0xFFF);

    // Entferne Mapping
    pt->entries[pt_idx] = 0;

    // Invalidiere TLB
    __asm__ volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");
}

u64 virtmem_get_physical(pml4_t* pml4, u64 virt_addr)
{
    if (pml4 == NULL) {
        return 0;
    }

    u64 pml4_idx = (virt_addr >> 39) & 0x1FF;
    u64 pdpt_idx = (virt_addr >> 30) & 0x1FF;
    u64 pd_idx = (virt_addr >> 21) & 0x1FF;
    u64 pt_idx = (virt_addr >> 12) & 0x1FF;
    u64 offset = virt_addr & 0xFFF;

    if (!(pml4->entries[pml4_idx] & PAGE_PRESENT)) return 0;
    pdpt_t* pdpt = (pdpt_t*)(pml4->entries[pml4_idx] & ~0xFFF);

    if (!(pdpt->entries[pdpt_idx] & PAGE_PRESENT)) return 0;
    pd_t* pd = (pd_t*)(pdpt->entries[pdpt_idx] & ~0xFFF);

    if (!(pd->entries[pd_idx] & PAGE_PRESENT)) return 0;
    pt_t* pt = (pt_t*)(pd->entries[pd_idx] & ~0xFFF);

    if (!(pt->entries[pt_idx] & PAGE_PRESENT)) return 0;

    return (pt->entries[pt_idx] & ~0xFFF) | offset;
}

void virtmem_switch_context(pml4_t* pml4)
{
    if (pml4 != NULL) {
        load_cr3((u64)pml4);
    }
}
