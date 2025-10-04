#include "vmalloc.h"
#include "physmem.h"
#include "virtmem.h"
#include "../../libs/memory/main.h"

static vmalloc_region_t* kernel_regions = NULL;
static u64 kernel_next_addr = KERNEL_HEAP_START;

void vmalloc_init(void)
{
    kernel_regions = NULL;
    kernel_next_addr = KERNEL_HEAP_START;
}

static vmalloc_region_t* find_region(vmalloc_region_t** list, void* ptr)
{
    u64 addr = (u64)ptr;
    vmalloc_region_t* current = *list;

    while (current != NULL) {
        if (current->virt_start == addr) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

void* kvmalloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    // Runde auf Seitengröße auf
    size_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t total_size = pages_needed * PAGE_SIZE;

    // Prüfe ob genug virtueller Adressraum verfügbar
    if (kernel_next_addr + total_size >= KERNEL_HEAP_START + KERNEL_HEAP_SIZE) {
        return NULL;
    }

    u64 virt_addr = kernel_next_addr;
    kernel_next_addr += total_size;

    // Allokiere physische Seiten und mappe sie
    for (size_t i = 0; i < pages_needed; i++) {
        u64 phys_page = physmem_alloc_page();
        if (phys_page == 0) {
            // Fehler - gebe bereits allokierte Seiten frei
            for (size_t j = 0; j < i; j++) {
                u64 virt = virt_addr + (j * PAGE_SIZE);
                u64 phys = virtmem_get_physical(NULL, virt);
                physmem_free_page(phys);
            }
            kernel_next_addr -= total_size;
            return NULL;
        }

        // Mappe Seite (NULL = verwende Kernel PML4)
        virtmem_map_page(NULL, virt_addr + (i * PAGE_SIZE), phys_page,
                        PAGE_WRITE);
    }

    // Erstelle Region-Entry
    vmalloc_region_t* region = (vmalloc_region_t*)kalloc(sizeof(vmalloc_region_t));
    if (region == NULL) {
        // Fehler - cleanup
        for (size_t i = 0; i < pages_needed; i++) {
            u64 virt = virt_addr + (i * PAGE_SIZE);
            u64 phys = virtmem_get_physical(NULL, virt);
            physmem_free_page(phys);
            virtmem_unmap_page(NULL, virt);
        }
        kernel_next_addr -= total_size;
        return NULL;
    }

    region->virt_start = virt_addr;
    region->size = total_size;
    region->flags = PAGE_WRITE;
    region->next = kernel_regions;
    kernel_regions = region;

    // Lösche Speicher
    memset((void*)virt_addr, 0, total_size);

    return (void*)virt_addr;
}

void kvfree(void* ptr)
{
    if (ptr == NULL) {
        return;
    }

    u64 addr = (u64)ptr;

    // Finde Region
    vmalloc_region_t* prev = NULL;
    vmalloc_region_t* current = kernel_regions;

    while (current != NULL) {
        if (current->virt_start == addr) {
            break;
        }
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        return; // Region nicht gefunden
    }

    // Gebe physische Seiten frei
    size_t pages = current->size / PAGE_SIZE;
    for (size_t i = 0; i < pages; i++) {
        u64 virt = current->virt_start + (i * PAGE_SIZE);
        u64 phys = virtmem_get_physical(NULL, virt);
        if (phys != 0) {
            physmem_free_page(phys);
        }
        virtmem_unmap_page(NULL, virt);
    }

    // Entferne Region aus Liste
    if (prev == NULL) {
        kernel_regions = current->next;
    } else {
        prev->next = current->next;
    }

    kfree(current);
}

void* uvmalloc(pml4_t* pml4, size_t size)
{
    if (pml4 == NULL || size == 0) {
        return NULL;
    }

    size_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t total_size = pages_needed * PAGE_SIZE;

    // Finde freien Bereich im User-Space
    u64 virt_addr = USER_HEAP_START;

    // Allokiere physische Seiten und mappe sie
    for (size_t i = 0; i < pages_needed; i++) {
        u64 phys_page = physmem_alloc_page();
        if (phys_page == 0) {
            // Cleanup bei Fehler
            for (size_t j = 0; j < i; j++) {
                u64 virt = virt_addr + (j * PAGE_SIZE);
                u64 phys = virtmem_get_physical(pml4, virt);
                physmem_free_page(phys);
                virtmem_unmap_page(pml4, virt);
            }
            return NULL;
        }

        virtmem_map_page(pml4, virt_addr + (i * PAGE_SIZE), phys_page,
                        PAGE_WRITE | PAGE_USER);
    }

    return (void*)virt_addr;
}

void uvfree(pml4_t* pml4, void* ptr)
{
    if (pml4 == NULL || ptr == NULL) {
        return;
    }

    u64 addr = (u64)ptr;

    // Finde heraus wie viele Seiten allokiert wurden
    // (vereinfachte Version - in Produktion würde man dies tracken)
    u64 phys = virtmem_get_physical(pml4, addr);
    if (phys != 0) {
        physmem_free_page(phys);
        virtmem_unmap_page(pml4, addr);
    }
}

void vmalloc_dump_regions(void)
{
    // Debug-Funktion zum Ausgeben aller Regionen
    vmalloc_region_t* current = kernel_regions;

    while (current != NULL) {
        // Hier könnte man print() verwenden
        current = current->next;
    }
}
