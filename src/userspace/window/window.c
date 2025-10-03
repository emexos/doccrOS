#include "window.h"
#include "../../libs/graphics/graphics.h"
#include "../../libs/graphics/colors/stdclrs.h"
#include "../../libs/print/print.h"
#include "../../libs/string/string.h"

static window_t window = {0};
static int drawing_window = 0;
static int window_start_x = 0;
static int window_start_y = 0;

void window_init(void) {
    window.active = 0;
    drawing_window = 0;
    window_start_x = 0;
    window_start_y = 0;
}

void window_draw(window_t *win) {
    if (!win->active) return;

    // window background
    draw_rect(win->x, win->y, win->width, win->height, 0xFFD8DEE9);

    // window border (2px thick)
    // top
    draw_rect(win->x, win->y, win->width, 2, 0xFF4C566A);
    // bottom
    draw_rect(win->x, win->y + win->height - 2, win->width, 2, 0xFF4C566A);
    // left
    draw_rect(win->x, win->y, 2, win->height, 0xFF4C566A);
    // right
    draw_rect(win->x + win->width - 2, win->y, 2, win->height, 0xFF4C566A);
}

void window_start_draw(int x, int y) {
    if (window.active) return; // only one window allowed

    drawing_window = 1;
    window_start_x = x;
    window_start_y = y;
}

void window_finish_draw(int end_x, int end_y) {
    if (!drawing_window) return;

    drawing_window = 0;

    int width = end_x - window_start_x;
    int height = end_y - window_start_y;

    // handle negative dimensions (dragging in opposite direction)
    int final_x = window_start_x;
    int final_y = window_start_y;

    if (width < 0) {
        final_x = end_x;
        width = -width;
    }
    if (height < 0) {
        final_y = end_y;
        height = -height;
    }

    // ensure minimum size
    if (width < 100) width = 100;
    if (height < 80) height = 80;

    window.x = final_x;
    window.y = final_y;
    window.width = width;
    window.height = height;
    window.active = 1;
}

void window_draw_preview(int start_x, int start_y, int end_x, int end_y) {
    int width = end_x - start_x;
    int height = end_y - start_y;

    // handle negative dimensions
    int x = start_x;
    int y = start_y;

    if (width < 0) {
        x = end_x;
        width = -width;
    }
    if (height < 0) {
        y = end_y;
        height = -height;
    }

    if (width > 5 && height > 5) {
        // draw preview outline (dashed style)
        // top
        for (int i = 0; i < width; i += 10) {
            draw_rect(x + i, y, 5, 1, GFX_WHITE);
        }
        // bottom
        for (int i = 0; i < width; i += 10) {
            draw_rect(x + i, y + height, 5, 1, GFX_WHITE);
        }
        // left
        for (int i = 0; i < height; i += 10) {
            draw_rect(x, y + i, 1, 5, GFX_WHITE);
        }
        // right
        for (int i = 0; i < height; i += 10) {
            draw_rect(x + width, y + i, 1, 5, GFX_WHITE);
        }
    }
}

window_t* window_get_current(void) {
    return &window;
}

int window_is_drawing(void) {
    return drawing_window;
}

int window_get_start_x(void) {
    return window_start_x;
}

int window_get_start_y(void) {
    return window_start_y;
}
