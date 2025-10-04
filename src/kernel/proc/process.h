#ifndef PROCESS_H
#define PROCESS_H

#include "../../../shared/types.h"
#include "../cpu/idt.h"
#include "../mem_manager/virtmem.h"

#define MAX_PROCESSES 256
#define KERNEL_STACK_SIZE 8192
#define USER_STACK_SIZE 8192

// Process States
typedef enum {
    PROC_STATE_READY,
    PROC_STATE_RUNNING,
    PROC_STATE_WAITING,
    PROC_STATE_BLOCKED,
    PROC_STATE_TERMINATED
} process_state_t;

// Process Priority
typedef enum {
    PROC_PRIORITY_IDLE = 0,
    PROC_PRIORITY_LOW = 1,
    PROC_PRIORITY_NORMAL = 2,
    PROC_PRIORITY_HIGH = 3,
    PROC_PRIORITY_REALTIME = 4
} process_priority_t;

// Process Structure
typedef struct process {
    u64 pid;
    char name[64];

    // CPU State
    cpu_state_t cpu_state;

    // Memory
    pml4_t* page_table;
    u64 kernel_stack;
    u64 user_stack;

    // State
    process_state_t state;
    process_priority_t priority;

    // Timing
    u64 time_slice;
    u64 time_used;
    u64 total_time;

    // Linked List
    struct process* next;
    struct process* prev;

    // Parent/Child
    struct process* parent;
    struct process* first_child;

    // Exit Code
    int exit_code;

} process_t;

// Process Management
void process_init(void);
process_t* process_create(const char* name, void (*entry_point)(void), int is_kernel);
void process_destroy(process_t* proc);
void process_exit(int exit_code);

// Process State Management
void process_block(process_t* proc, process_state_t new_state);
void process_unblock(process_t* proc);
void process_sleep(u64 milliseconds);

// Process Information
process_t* process_get_by_pid(u64 pid);
u64 process_get_current_pid(void);

#endif
