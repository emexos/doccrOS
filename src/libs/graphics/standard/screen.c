#include <stdint.h>
#include "../graphics.h"

void clear(uint32_t color)
{
    uint32_t *fb = get_framebuffer();
    uint32_t width = get_fb_width();
    uint32_t height = get_fb_height();

    for (uint32_t i = 0; i < width * height; i++)
    {
        fb[i] = color;
    }
    reset_cursor();
    //
}
