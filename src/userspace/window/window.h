#ifndef WINDOW_H
#define WINDOW_H

#include "../../../shared/types.h"

typedef struct {
    int x;
    int y;
    int width;
    int height;
    u32 color;
    const char *title;
    int active;
} window_t;

void window_init(void);
void window_draw(window_t *win);
void window_start_draw(int x, int y);
void window_finish_draw(int x, int y);
void window_draw_preview(int start_x, int start_y, int end_x, int end_y);
window_t* window_get_current(void);
int window_is_drawing(void);
int window_get_start_x(void);
int window_get_start_y(void);

#endif
