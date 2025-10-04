#ifndef TIMER_H
#define TIMER_H

#include "../../../shared/types.h"

#define TIMER_FREQUENCY 1000  // 1000 Hz = 1ms Ticks

// Timer Functions
void timer_init(u32 frequency);
void timer_wait(u32 ticks);
u64 timer_get_ticks(void);
u64 timer_get_seconds(void);

// Timer Handler (wird von IRQ0 aufgerufen)
void timer_handler_internal(void);

#endif
