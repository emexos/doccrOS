#include <stdint.h>
#include <limine.h>
#include "../fonts/font.h"
#include "colors/stdclrs.h"
#include "graphics.h"
#include "standard/screen.h"
#include "../string/string.h"

typedef unsigned long size_t;
#define NULL ((void*)0)

static uint32_t *framebuffer = NULL;
static uint32_t fb_width = 0;
static uint32_t fb_height = 0;
static uint32_t fb_pitch = 0;
static uint32_t cursor_x = 10;
static uint32_t cursor_y = 10;

void graphics_init(struct limine_framebuffer *fb)
{
    framebuffer = (uint32_t *)fb->address;
    fb_width = fb->width;
    fb_height = fb->height;
    fb_pitch = fb->pitch;
    cursor_x = 10;
    cursor_y = 10;

    print("Graphics", GFX_BLUE);
    char res_buf[64];
    str_copy(res_buf, "Framebuffer ");
    str_append_uint(res_buf, fb_width);
    str_append(res_buf, "x");
    str_append_uint(res_buf, fb_height);
    print(res_buf, GFX_BLUE);
}

void putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x < fb_width && y < fb_height) {
        framebuffer[y * (fb_pitch / 4) + x] = color;
    }
}

static void putchar_at(char c, uint32_t x, uint32_t y, uint32_t color)
{
    const uint8_t *glyph = font_8x8[(uint8_t)c];
    for (int dy = 0; dy < 8; dy++)
    {
        uint8_t row = glyph[dy];
        for (int dx = 0; dx < 8; dx++)
        {
            if (row & (1 << (7 - dx)))
            {
                putpixel(x + dx, y + dy, color);
            }
        }
    }
}

void putchar(char c, uint32_t color)
{
    if (c == '\n')
    {
        cursor_x = 10;
        cursor_y += 10;
        return;
    }

    if (cursor_x + 8 >= fb_width)
    {
        cursor_x = 10;
        cursor_y += 10;
    }

    if (cursor_y + 8 >= fb_height)
    {
        cursor_y = 10;
    }

    putchar_at(c, cursor_x, cursor_y, color);
    cursor_x += 9;
}

void string(const char *str, uint32_t color)
{
    for (size_t i = 0; str[i]; i++)
    {
        putchar(str[i], color);
    }
}

void print(const char *str, uint32_t color)
{
    string(str, color);
    putchar('\n', color);
}

uint32_t get_fb_width(void)
{
    return fb_width;
}

uint32_t get_fb_height(void)
{
    return fb_height;
}

uint32_t* get_framebuffer(void)
{
    return framebuffer;
}

uint32_t get_fb_pitch(void)
{
    return fb_pitch;
}

void reset_cursor(void)
{
    cursor_x = 10;
    cursor_y = 10;
}
