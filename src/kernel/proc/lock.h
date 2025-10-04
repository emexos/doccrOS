#ifndef LOCK_H
#define LOCK_H

#include "../../../shared/types.h"
#include "process.h"

// Mutex Structure
typedef struct mutex {
    volatile int locked;
    u32 locked_count;
    struct process* owner;
    struct process* first_waiting_list;
    struct process* last_waiting_list;
} mutex_t;

// Spinlock Structure
typedef struct {
    volatile int locked;
} spinlock_t;

// Mutex Functions
mutex_t* create_mutex(void);
void destroy_mutex(mutex_t* mut);
void acquire_mutex(mutex_t* mut);
void release_mutex(mutex_t* mut);

// Spinlock Functions
void spinlock_init(spinlock_t* lock);
void spinlock_acquire(spinlock_t* lock);
void spinlock_release(spinlock_t* lock);
int spinlock_try_acquire(spinlock_t* lock);

// Scheduler Lock
void lock_scheduler(void);
void unlock_scheduler(void);

#endif
