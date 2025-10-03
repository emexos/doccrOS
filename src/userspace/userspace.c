#include "userspace.h"
#include "window/window.h"
#include "../libs/graphics/graphics.h"
#include "../libs/graphics/colors/stdclrs.h"
#include "../libs/print/print.h"
#include "../libs/string/string.h"
#include "../drivers/ps2/ps2.h"
#include "../kernel/console/console.h"
#include "../../shared/theme/doccr.h"

static int last_mouse_x = 0;
static int last_mouse_y = 0;
static int last_button_state = 0;
static int in_window_console = 0;

static void draw_mouse(int x, int y) {
    // draw simple mouse cursor (arrow style)
    for (int i = 0; i < 10; i++) {
        putpixel(x, y + i, IMOUSE_COLOR);
        if (i < 10) putpixel(x + i, y, IMOUSE_COLOR);
    }
    for (int i = 1; i < 3; i++) {
        putpixel(x + i, y + i, OMOUSE_COLOR);
    }
}

static void redraw_area(int x, int y, int w, int h) {
    // redraw background
    draw_rect(x, y, w, h, BACKGROUND_COLOR);

    // redraw window if in area
    window_t *win = window_get_current();
    if (win->active) {
        if (!(x > win->x + win->width || x + w < win->x ||
              y > win->y + win->height || y + h < win->y)) {
            window_draw(win);
        }
    }
}

void userspace_init(void) {
    clear(BACKGROUND_COLOR);

    window_init();
    in_window_console = 0;

    // init mouse position
    mouse_state_t *mouse = mouse_get_state();
    last_mouse_x = mouse->x;
    last_mouse_y = mouse->y;
    last_button_state = 0;

    draw_mouse(mouse->x, mouse->y);
}

void userspace_run(void) {
    while (1) {
        keyboard_poll();
        mouse_poll();

        mouse_state_t *mouse = mouse_get_state();
        window_t *win = window_get_current();

        // redraw mouse if moved
        if (mouse->x != last_mouse_x || mouse->y != last_mouse_y) {
            // clear old mouse
            redraw_area(last_mouse_x, last_mouse_y, 11, 11);

            // redraw preview if drawing
            if (window_is_drawing()) {
                window_draw_preview(window_get_start_x(), window_get_start_y(),
                                   mouse->x, mouse->y);
            }

            draw_mouse(mouse->x, mouse->y);
            last_mouse_x = mouse->x;
            last_mouse_y = mouse->y;
        }

        // handle window drawing
        if (mouse->left_button && !last_button_state && !win->active) {
            // start drawing window (button pressed)
            window_start_draw(mouse->x, mouse->y);
        }

        if (!mouse->left_button && last_button_state && window_is_drawing()) {
            // finish drawing window (button released)
            window_finish_draw(mouse->x, mouse->y);

            // clear preview and redraw final window
            clear(BACKGROUND_COLOR);
            window_draw(win);
            draw_mouse(mouse->x, mouse->y);

            // start console in window
            in_window_console = 1;
        }

        // draw preview while dragging
        if (window_is_drawing() && mouse->left_button) {
            window_draw_preview(window_get_start_x(), window_get_start_y(),
                               mouse->x, mouse->y);
            draw_mouse(mouse->x, mouse->y);
        }

        last_button_state = mouse->left_button;

        // handle keyboard in window console
        if (in_window_console && keyboard_has_key()) {
            char c = keyboard_get_key();
            console_handle_key(c);
        }
    }
}
