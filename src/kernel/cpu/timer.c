#include "timer.h"
#include "irq.h"
#include "../proc/scheduler.h"

static volatile u64 timer_ticks = 0;

static inline void outb(u16 port, u8 val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void timer_handler(cpu_state_t* state)
{
    (void)state;
    timer_ticks++;

    // Rufe Scheduler Tick auf
    scheduler_tick();
}

void timer_init(u32 frequency)
{
    // Registriere Timer IRQ Handler
    irq_register_handler(0, timer_handler);

    // Berechne Divisor für PIT
    u32 divisor = 1193182 / frequency;

    // Sende Command Byte
    outb(0x43, 0x36);  // Channel 0, lo/hi byte, rate generator

    // Sende Frequency Divisor
    outb(0x40, divisor & 0xFF);         // Low byte
    outb(0x40, (divisor >> 8) & 0xFF);  // High byte

    timer_ticks = 0;
}

void timer_wait(u32 ticks)
{
    u64 start = timer_ticks;
    while (timer_ticks < start + ticks) {
        __asm__ volatile("hlt");
    }
}

u64 timer_get_ticks(void)
{
    return timer_ticks;
}

u64 timer_get_seconds(void)
{
    return timer_ticks / TIMER_FREQUENCY;
}

void timer_handler_internal(void)
{
    timer_ticks++;
    scheduler_tick();
}
