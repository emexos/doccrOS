#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>

void draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void draw_circle(uint32_t cx, uint32_t cy, uint32_t radius, uint32_t color);
void draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color);

#endif
