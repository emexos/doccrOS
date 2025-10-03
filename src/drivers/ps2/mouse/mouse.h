#ifndef MOUSE_H
#define MOUSE_H

#include "../../../../shared/types.h"

typedef struct {
    int x;
    int y;
    int left_button;
    int right_button;
    int middle_button;
} mouse_state_t;

void mouse_init(void);
void mouse_poll(void);
mouse_state_t* mouse_get_state(void);

#endif
