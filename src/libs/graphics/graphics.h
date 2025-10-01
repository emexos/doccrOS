#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <limine.h>

void graphics_init(struct limine_framebuffer *fb);
void putpixel(uint32_t x, uint32_t y, uint32_t color);
void putchar(char c, uint32_t color);
void string(const char *str, uint32_t color);
void print(const char *str, uint32_t color);
uint32_t get_fb_width(void);
uint32_t get_fb_height(void);
uint32_t* get_framebuffer(void);
uint32_t get_fb_pitch(void);
void reset_cursor(void);

#endif
