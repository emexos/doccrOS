#include "process.h"
#include "scheduler.h"
#include "lock.h"
#include "../../libs/memory/main.h"
#include "../mem_manager/physmem.h"
#include "../mem_manager/virtmem.h"

static process_t* process_table[MAX_PROCESSES];
static u64 next_pid = 1;

void process_init(void)
{
    // Initialisiere Prozess-Tabelle
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i] = NULL;
    }

    next_pid = 1;
}

process_t* process_create(const char* name, void (*entry_point)(void), int is_kernel)
{
    // Finde freien Slot in Prozess-Tabelle
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] == NULL) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        return NULL; // Keine freien Slots
    }

    // Allokiere Prozess-Struktur
    process_t* proc = (process_t*)kalloc(sizeof(process_t));
    if (proc == NULL) {
        return NULL;
    }

    // Initialisiere Prozess
    proc->pid = next_pid++;

    // Kopiere Name
    int i = 0;
    while (name[i] != '\0' && i < 63) {
        proc->name[i] = name[i];
        i++;
    }
    proc->name[i] = '\0';

    // Erstelle Page Table
    if (is_kernel) {
        proc->page_table = NULL; // Kernel verwendet seine eigene Page Table
    } else {
        proc->page_table = virtmem_create_address_space();
        if (proc->page_table == NULL) {
            kfree(proc);
            return NULL;
        }
    }

    // Allokiere Kernel Stack
    proc->kernel_stack = (u64)kalloc(KERNEL_STACK_SIZE);
    if (proc->kernel_stack == 0) {
        if (proc->page_table != NULL) {
            virtmem_destroy_address_space(proc->page_table);
        }
        kfree(proc);
        return NULL;
    }

    // Allokiere User Stack (falls User-Prozess)
    if (!is_kernel) {
        proc->user_stack = (u64)kalloc(USER_STACK_SIZE);
        if (proc->user_stack == 0) {
            kfree((void*)proc->kernel_stack);
            virtmem_destroy_address_space(proc->page_table);
            kfree(proc);
            return NULL;
        }
    } else {
        proc->user_stack = 0;
    }

    // Initialisiere CPU State
    memset(&proc->cpu_state, 0, sizeof(cpu_state_t));
    proc->cpu_state.rip = (u64)entry_point;
    proc->cpu_state.rsp = proc->kernel_stack + KERNEL_STACK_SIZE - 16;
    proc->cpu_state.cs = 0x08; // Kernel Code Segment
    proc->cpu_state.ss = 0x10; // Kernel Data Segment
    proc->cpu_state.rflags = 0x202; // Interrupts enabled

    // Prozess State
    proc->state = PROC_STATE_READY;
    proc->priority = PROC_PRIORITY_NORMAL;

    // Timing
    proc->time_slice = 10; // 10ms
    proc->time_used = 0;
    proc->total_time = 0;

    // Listen
    proc->next = NULL;
    proc->prev = NULL;
    proc->parent = NULL;
    proc->first_child = NULL;

    proc->exit_code = 0;

    // Füge zu Prozess-Tabelle hinzu
    process_table[slot] = proc;

    // Füge zu Scheduler hinzu
    scheduler_add_process(proc);

    return proc;
}

void process_destroy(process_t* proc)
{
    if (proc == NULL) {
        return;
    }

    // Entferne aus Scheduler
    scheduler_remove_process(proc);

    // Gebe Speicher frei
    if (proc->kernel_stack != 0) {
        kfree((void*)proc->kernel_stack);
    }

    if (proc->user_stack != 0) {
        kfree((void*)proc->user_stack);
    }

    if (proc->page_table != NULL) {
        virtmem_destroy_address_space(proc->page_table);
    }

    // Entferne aus Prozess-Tabelle
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] == proc) {
            process_table[i] = NULL;
            break;
        }
    }

    kfree(proc);
}

void process_exit(int exit_code)
{
    process_t* current = scheduler_get_current();
    if (current == NULL) {
        return;
    }

    current->exit_code = exit_code;
    current->state = PROC_STATE_TERMINATED;

    // Wechsle zum nächsten Prozess
    scheduler_yield();
}

void process_block(process_t* proc, process_state_t new_state)
{
    if (proc != NULL) {
        proc->state = new_state;
    }
}

void process_unblock(process_t* proc)
{
    if (proc != NULL) {
        proc->state = PROC_STATE_READY;
    }
}

void process_sleep(u64 milliseconds)
{
    // Vereinfachte Sleep-Implementierung
    // In echter Implementation würde man einen Timer verwenden
    (void)milliseconds;

    process_t* current = scheduler_get_current();
    if (current != NULL) {
        process_block(current, PROC_STATE_WAITING);
        scheduler_yield();
    }
}

process_t* process_get_by_pid(u64 pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] != NULL && process_table[i]->pid == pid) {
            return process_table[i];
        }
    }
    return NULL;
}

u64 process_get_current_pid(void)
{
    process_t* current = scheduler_get_current();
    if (current != NULL) {
        return current->pid;
    }
    return 0;
}
