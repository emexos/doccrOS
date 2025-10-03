#include "mouse.h"
#include "../../../libs/graphics/graphics.h"

static inline u8 inb(u16 port) {
    u8 ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(u16 port, u8 val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static mouse_state_t mouse_state = {0, 0, 0, 0, 0};
static u8 mouse_cycle = 0;
static u8 mouse_byte[3];

static void mouse_wait(u8 type) {
    u32 timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if ((inb(0x64) & 1) == 1) return;
        }
    } else {
        while (timeout--) {
            if ((inb(0x64) & 2) == 0) return;
        }
    }
}

static void mouse_write(u8 data) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, data);
}

static u8 mouse_read(void) {
    mouse_wait(0);
    return inb(0x60);
}

void mouse_init(void) {
    mouse_state.x = fb_width / 2;
    mouse_state.y = fb_height / 2;
    mouse_state.left_button = 0;
    mouse_state.right_button = 0;
    mouse_state.middle_button = 0;
    mouse_cycle = 0;

    // enable auxiliary device
    mouse_wait(1);
    outb(0x64, 0xA8);

    // get compaq status byte
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    u8 status = (inb(0x60) | 2);

    // set compaq status byte
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);

    // use default settings
    mouse_write(0xF6);
    mouse_read();

    // enable data reporting
    mouse_write(0xF4);
    mouse_read();
}

void mouse_poll(void) {
    // check if data available
    u8 status = inb(0x64);
    if ((status & 1) == 0) return;
    if ((status & 0x20) == 0) return;

    u8 data = inb(0x60);

    switch(mouse_cycle) {
        case 0:
            mouse_byte[0] = data;
            // validate first byte
            if ((mouse_byte[0] & 0x08) == 0) {
                return; // invalid packet
            }
            mouse_cycle++;
            break;
        case 1:
            mouse_byte[1] = data;
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = data;
            mouse_cycle = 0;

            // parse mouse packet
            mouse_state.left_button = mouse_byte[0] & 0x1;
            mouse_state.right_button = (mouse_byte[0] & 0x2) >> 1;
            mouse_state.middle_button = (mouse_byte[0] & 0x4) >> 2;

            // get movement
            int dx = mouse_byte[1];
            int dy = mouse_byte[2];

            // sign extend
            if (mouse_byte[0] & 0x10) dx -= 256;
            if (mouse_byte[0] & 0x20) dy -= 256;

            // update position
            mouse_state.x += dx;
            mouse_state.y -= dy; // invert Y

            // clamp to screen
            if (mouse_state.x < 0) mouse_state.x = 0;
            if (mouse_state.y < 0) mouse_state.y = 0;
            if (mouse_state.x >= (int)fb_width) mouse_state.x = fb_width - 1;
            if (mouse_state.y >= (int)fb_height) mouse_state.y = fb_height - 1;
            break;
    }
}

mouse_state_t* mouse_get_state(void) {
    return &mouse_state;
}
