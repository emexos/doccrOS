#include <stdint.h>
#include <limine.h>
#include "../libs/graphics/graphics.h"
#include "../libs/graphics/standard/screen.h"
#include "../libs/graphics/draw.h"
#include "../libs/graphics/colors/stdclrs.h"
#include "../libs/string/string.h"

//NULL standart
#define NULL ((void*)0)

// Limine Requests
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Halt
static void hcf(void)
{
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

// Simple delay function
static void delay(uint32_t count)
{
    for (volatile uint32_t i = 0; i < count * 10000000; i++)
    {
        __asm__ volatile ("nop");
    }
}

//entry
void _start(void)
{
    // emexOS Kernel

    // Check if Limine protocol is supported
    if (LIMINE_BASE_REVISION_SUPPORTED == 0) {
        hcf(); //if not halt
    }

    // framebuffer from limine protocol
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf(); //if not halt
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    graphics_init(fb);

    // Boot messages stay for a moment just for a good look
    delay(30);

    // Clear screen after boot
    clear(GFX_BLACK);

    //shell_init();
    // there isnt a shell yet
    print_str("screentest", GFX_GREEN);

    // Test draw functions
    draw_rect(50, 50, 100, 60, GFX_RED);
    draw_circle(400, 100, 40, GFX_YELLOW);
    draw_line(50, 350, 300, 400, GFX_PURPLE);

    // Should never reach here
    hcf();
}
