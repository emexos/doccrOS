#include "scheduler.h"
#include "process.h"
#include "lock.h"
#include "../cpu/idt.h"
#include "../mem_manager/virtmem.h"
#include "../../libs/memory/main.h"

static process_t* ready_queue_head = NULL;
static process_t* ready_queue_tail = NULL;
static process_t* current_process = NULL;
static int scheduler_enabled = 0;
static scheduler_algorithm_t algorithm = SCHED_ROUND_ROBIN;

static u64 context_switches = 0;
static u64 active_processes = 0;

// Externes Context Switch aus Assembly
extern void scheduler_context_switch(cpu_state_t* old_state, cpu_state_t* new_state);

void scheduler_init(void)
{
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    current_process = NULL;
    scheduler_enabled = 0;
    context_switches = 0;
    active_processes = 0;
    algorithm = SCHED_ROUND_ROBIN;
}

void scheduler_start(void)
{
    if (ready_queue_head == NULL) {
        return; // Keine Prozesse zum Starten
    }

    scheduler_enabled = 1;
    current_process = ready_queue_head;
    current_process->state = PROC_STATE_RUNNING;

    // Wechsle zu erstem Prozess
    if (current_process->page_table != NULL) {
        virtmem_switch_context(current_process->page_table);
    }
}

void scheduler_add_process(process_t* proc)
{
    if (proc == NULL) {
        return;
    }

    lock_scheduler();

    // Füge zur Ready Queue hinzu
    proc->next = NULL;
    proc->prev = ready_queue_tail;

    if (ready_queue_tail != NULL) {
        ready_queue_tail->next = proc;
    }
    ready_queue_tail = proc;

    if (ready_queue_head == NULL) {
        ready_queue_head = proc;
    }

    active_processes++;

    unlock_scheduler();
}

void scheduler_remove_process(process_t* proc)
{
    if (proc == NULL) {
        return;
    }

    lock_scheduler();

    // Entferne aus Ready Queue
    if (proc->prev != NULL) {
        proc->prev->next = proc->next;
    } else {
        ready_queue_head = proc->next;
    }

    if (proc->next != NULL) {
        proc->next->prev = proc->prev;
    } else {
        ready_queue_tail = proc->prev;
    }

    if (active_processes > 0) {
        active_processes--;
    }

    unlock_scheduler();
}

process_t* scheduler_get_next(void)
{
    process_t* next = NULL;

    switch (algorithm) {
        case SCHED_ROUND_ROBIN:
            // Round Robin: Wähle nächsten READY Prozess
            next = current_process ? current_process->next : ready_queue_head;

            // Suche READY Prozess
            while (next != NULL && next->state != PROC_STATE_READY) {
                next = next->next;
            }

            // Wrap around
            if (next == NULL) {
                next = ready_queue_head;
                while (next != NULL && next->state != PROC_STATE_READY) {
                    next = next->next;
                }
            }
            break;

        case SCHED_PRIORITY:
            // Priority: Wähle höchste Priorität
            {
                process_t* candidate = ready_queue_head;
                process_priority_t highest = PROC_PRIORITY_IDLE;

                while (candidate != NULL) {
                    if (candidate->state == PROC_STATE_READY &&
                        candidate->priority > highest) {
                        highest = candidate->priority;
                        next = candidate;
                    }
                    candidate = candidate->next;
                }
            }
            break;

        case SCHED_MULTILEVEL_QUEUE:
            // Multilevel Queue mit Aging
            // TODO: Implementierung für komplexere Scheduling-Strategie
            next = scheduler_get_next(); // Fallback zu Round Robin
            break;
    }

    return next;
}

void scheduler_yield(void)
{
    if (!scheduler_enabled || current_process == NULL) {
        return;
    }

    lock_scheduler();

    process_t* next = scheduler_get_next();

    if (next == NULL || next == current_process) {
        unlock_scheduler();
        return;
    }

    // Speichere aktuellen Prozess State
    process_t* old_process = current_process;
    if (old_process->state == PROC_STATE_RUNNING) {
        old_process->state = PROC_STATE_READY;
    }

    // Setze neuen Prozess
    current_process = next;
    current_process->state = PROC_STATE_RUNNING;
    context_switches++;

    // Wechsle Page Table falls User-Prozess
    if (current_process->page_table != NULL) {
        virtmem_switch_context(current_process->page_table);
    }

    unlock_scheduler();

    // Führe Context Switch durch
    scheduler_context_switch(&old_process->cpu_state, &current_process->cpu_state);
}

void scheduler_tick(void)
{
    if (!scheduler_enabled || current_process == NULL) {
        return;
    }

    current_process->time_used++;
    current_process->total_time++;

    // Prüfe ob Time Slice abgelaufen
    if (current_process->time_used >= current_process->time_slice) {
        current_process->time_used = 0;
        scheduler_yield();
    }
}

process_t* scheduler_get_current(void)
{
    return current_process;
}

void scheduler_switch_to(process_t* proc)
{
    if (proc == NULL || !scheduler_enabled) {
        return;
    }

    lock_scheduler();

    if (current_process != NULL) {
        if (current_process->state == PROC_STATE_RUNNING) {
            current_process->state = PROC_STATE_READY;
        }
    }

    process_t* old = current_process;
    current_process = proc;
    current_process->state = PROC_STATE_RUNNING;
    context_switches++;

    if (current_process->page_table != NULL) {
        virtmem_switch_context(current_process->page_table);
    }

    unlock_scheduler();

    if (old != NULL) {
        scheduler_context_switch(&old->cpu_state, &current_process->cpu_state);
    }
}

void scheduler_enable(void)
{
    scheduler_enabled = 1;
}

void scheduler_disable(void)
{
    scheduler_enabled = 0;
}

int scheduler_is_enabled(void)
{
    return scheduler_enabled;
}

u64 scheduler_get_context_switches(void)
{
    return context_switches;
}

u64 scheduler_get_active_processes(void)
{
    return active_processes;
}
