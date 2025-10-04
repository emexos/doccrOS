#ifndef PANIC_H
#define PANIC_H

#include "../../shared/types.h"
#include "cpu/idt.h"

// Panic Reasons
typedef enum {
    PANIC_GENERAL,
    PANIC_OUT_OF_MEMORY,
    PANIC_KERNEL_FAULT,
    PANIC_USERSPACE_FAULT,
    PANIC_SCHEDULER_ERROR,
    PANIC_HARDWARE_ERROR
} panic_reason_t;

// Panic Handler
void kernel_panic(const char* message, panic_reason_t reason);
void kernel_panic_cpu(const char* message, cpu_state_t* state);

// Userspace Crash Handler
void userspace_crash_handler(cpu_state_t* state);
void userspace_try_recovery(void);

// System Status
void system_enter_safe_mode(void);
int system_is_safe_mode(void);

#endif
