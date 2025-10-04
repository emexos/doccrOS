#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../../../shared/types.h"
#include "process.h"

// Scheduler Algorithms
typedef enum {
    SCHED_ROUND_ROBIN,
    SCHED_PRIORITY,
    SCHED_MULTILEVEL_QUEUE
} scheduler_algorithm_t;

// Scheduler Functions
void scheduler_init(void);
void scheduler_start(void);
void scheduler_add_process(process_t* proc);
void scheduler_remove_process(process_t* proc);
void scheduler_yield(void);
void scheduler_tick(void);

// Process Management
process_t* scheduler_get_current(void);
process_t* scheduler_get_next(void);
void scheduler_switch_to(process_t* proc);

// Scheduler Control
void scheduler_enable(void);
void scheduler_disable(void);
int scheduler_is_enabled(void);

// Statistics
u64 scheduler_get_context_switches(void);
u64 scheduler_get_active_processes(void);

#endif
