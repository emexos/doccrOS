#ifndef PS2_H
#define PS2_H

#include "../../../shared/types.h"
#include "mouse/mouse.h"

// keyboard driver
void keyboard_init(void);
void keyboard_poll(void);
int keyboard_has_key(void);
char keyboard_get_key(void);

// mouse driver
void mouse_init(void);
void mouse_poll(void);
mouse_state_t* mouse_get_state(void);

#endif
