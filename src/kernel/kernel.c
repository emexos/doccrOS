#include "include/assembly.h"
#include "../libs/graphics/graphics.h"
//#include "../libs/graphics/standard/screen.h"
//#include "../libs/graphics/draw.h"
#include "../libs/graphics/colors/stdclrs.h"
#include "../libs/string/string.h"
#include "../libs/memory/main.h"

#include "../drivers/ps2/keyboard/keyboard.h"

// 16MB heap
#define HEAP_SIZE (8 * 1024 * 1024)
static u8 kernel_heap[HEAP_SIZE] __attribute__((aligned(16)));

void main(void)
{
    // emexOS

    //delay(30);

    clear(GFX_BG);

    // init memory
    //mem_init(kernel_heap, HEAP_SIZE);

    //actually not needed but maybe later
    draw_rect(10, 10, fb_width - 20, fb_height - 20, GFX_BG);

    cursor_x = 20;
    cursor_y = 20;

    print("Memory Test:", GFX_CYAN);
    char buf[128];
    str_copy(buf, "Heap: ");
    str_append_uint(buf, HEAP_SIZE / 1024);
    str_append(buf, " KB");
    print(buf, GFX_CYAN);

    putchar('\n', GFX_WHITE);

    // run mem tests
    mem_test();

    putchar('\n', GFX_WHITE);

    // draw test
    print("Graphics Test", GFX_YELLOW);
    draw_rect(50, 350, 100, 60, GFX_RED);
    draw_circle(200, 380, 30, GFX_YELLOW);
    draw_line(300, 360, 400, 400, GFX_PURPLE);

    keyboard_poll();

    hcf();
}

void _start(void)
{
    // start os
    //

    if (LIMINE_BASE_REVISION_SUPPORTED == 0) {
        hcf();
    }

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    graphics_init(fb)
        // should draw a rect
    ;

    mem_init(kernel_heap, HEAP_SIZE);

    delay(30);

    main();
    // hcf(); cannot ever reach this
};
