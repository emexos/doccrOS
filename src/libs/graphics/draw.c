#include <stdint.h>
#include "graphics.h"
#include "draw.h"

void draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color)
{
    for (uint32_t dy = 0; dy < height; dy++)
    {
        for (uint32_t dx = 0; dx < width; dx++)
        {
            putpixel(x + dx, y + dy, color);
        }
    }
}

void draw_circle(uint32_t cx, uint32_t cy, uint32_t radius, uint32_t color)
{
    for (uint32_t y = 0; y <= radius; y++)
    {
        for (uint32_t x = 0; x <= radius; x++)
        {
            if (x * x + y * y <= radius * radius)
            {
                putpixel(cx + x, cy + y, color);
                putpixel(cx - x, cy + y, color);
                putpixel(cx + x, cy - y, color);
                putpixel(cx - x, cy - y, color);
            }
        }
    }
}

void draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (1)
    {
        putpixel(x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}
