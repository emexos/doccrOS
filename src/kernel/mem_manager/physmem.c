#include "physmem.h"
#include "../../libs/memory/main.h"

static physmem_manager_t phys_mem;

void physmem_init(u64 mem_size)
{
    // Initialisiere physischen Speicher-Manager
    phys_mem.total_pages = mem_size / PAGE_SIZE;

    if (phys_mem.total_pages > (PHYS_MEM_MAX / PAGE_SIZE)) {
        phys_mem.total_pages = PHYS_MEM_MAX / PAGE_SIZE;
    }

    // Markiere alle Seiten als frei
    memset(phys_mem.bitmap, 0, BITMAP_SIZE);

    phys_mem.free_pages = phys_mem.total_pages;
    phys_mem.used_pages = 0;

    // Reserviere erste 4MB für Kernel (1024 Seiten)
    for (u64 i = 0; i < 1024; i++) {
        u64 byte_idx = i / 8;
        u64 bit_idx = i % 8;
        phys_mem.bitmap[byte_idx] |= (1 << bit_idx);
    }

    phys_mem.free_pages -= 1024;
    phys_mem.used_pages += 1024;
}

u64 physmem_alloc_page(void)
{
    if (phys_mem.free_pages == 0) {
        return 0; // Kein freier Speicher
    }

    // Suche freie Seite in Bitmap
    for (u64 i = 0; i < phys_mem.total_pages; i++) {
        u64 byte_idx = i / 8;
        u64 bit_idx = i % 8;

        if (!(phys_mem.bitmap[byte_idx] & (1 << bit_idx))) {
            // Freie Seite gefunden
            phys_mem.bitmap[byte_idx] |= (1 << bit_idx);
            phys_mem.free_pages--;
            phys_mem.used_pages++;

            return i * PAGE_SIZE;
        }
    }

    return 0; // Keine freie Seite gefunden
}

void physmem_free_page(u64 phys_addr)
{
    u64 page_idx = phys_addr / PAGE_SIZE;

    if (page_idx >= phys_mem.total_pages) {
        return; // Ungültige Adresse
    }

    u64 byte_idx = page_idx / 8;
    u64 bit_idx = page_idx % 8;

    // Prüfe ob Seite bereits frei ist
    if (!(phys_mem.bitmap[byte_idx] & (1 << bit_idx))) {
        return; // Seite ist bereits frei (double free)
    }

    // Markiere Seite als frei
    phys_mem.bitmap[byte_idx] &= ~(1 << bit_idx);
    phys_mem.free_pages++;
    phys_mem.used_pages--;
}

u64 physmem_get_free_pages(void)
{
    return phys_mem.free_pages;
}

u64 physmem_get_used_pages(void)
{
    return phys_mem.used_pages;
}
