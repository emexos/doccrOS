#include "lock.h"
#include "process.h"
#include "scheduler.h"
#include "../../libs/memory/main.h"
#include "../../libs/print/print.h"
#include "../../libs/graphics/colors/stdclrs.h"
#include "../../libs/string/string.h"

static spinlock_t scheduler_lock = {0};

// === MUTEX IMPLEMENTATION ===

mutex_t* create_mutex(void)
{
    mutex_t* mut = (mutex_t*)kalloc(sizeof(mutex_t));
    if (mut == NULL) {
        return NULL;
    }

    mut->locked = 0;
    mut->locked_count = 0;
    mut->owner = NULL;
    mut->first_waiting_list = NULL;
    mut->last_waiting_list = NULL;

    return mut;
}

void destroy_mutex(mutex_t* mut)
{
    if (mut != NULL) {
        kfree(mut);
    }
}

void acquire_mutex(mutex_t* mut)
{
    if (mut == NULL) {
        return;
    }

    process_t* current_process = scheduler_get_current();

    if (mut->locked) {
        if (mut->owner == current_process) {
            // Mutex bereits von diesem Prozess gehalten
            mut->locked_count++;
            return;
        }

        // Prozess zur Warteliste hinzufügen
        lock_scheduler();
        current_process->next = NULL;

        if (mut->first_waiting_list == NULL) {
            mut->first_waiting_list = current_process;
            mut->last_waiting_list = current_process;
        } else {
            mut->last_waiting_list->next = current_process;
            mut->last_waiting_list = current_process;
        }

        unlock_scheduler();

        // Blockiere Task
        process_block(current_process, PROC_STATE_WAITING);
        scheduler_yield();
    } else {
        // Mutex ist frei
        mut->locked = 1;
        mut->owner = current_process;
        mut->locked_count = 0;
    }
}

void release_mutex(mutex_t* mut)
{
    if (mut == NULL) {
        return;
    }

    process_t* current_process = scheduler_get_current();

    if (mut->owner != current_process) {
        // Fehler: Prozess versucht Mutex freizugeben, den er nicht besitzt
        char buf[128];
        str_copy(buf, "Process ");
        str_append_uint(buf, (u32)current_process->pid);
        str_append(buf, " tried to release mutex it doesn't own!");
        print(buf, GFX_RED);
        return;
    }

    if (mut->locked_count != 0) {
        mut->locked_count--;
        return;
    }

    // Wecke wartenden Prozess auf
    if (mut->first_waiting_list != NULL) {
        lock_scheduler();

        process_t* released = mut->first_waiting_list;
        mut->first_waiting_list = mut->first_waiting_list->next;
        mut->owner = released;  // Neuer Besitzer

        process_unblock(released);

        unlock_scheduler();
    } else {
        // Keine wartenden Prozesse
        mut->locked = 0;
        mut->owner = NULL;
    }
}

// === SPINLOCK IMPLEMENTATION ===

void spinlock_init(spinlock_t* lock)
{
    if (lock != NULL) {
        lock->locked = 0;
    }
}

void spinlock_acquire(spinlock_t* lock)
{
    if (lock == NULL) {
        return;
    }

    while (__sync_lock_test_and_set(&lock->locked, 1)) {
        // Spin (aktiv warten)
        __asm__ volatile("pause");
    }
}

void spinlock_release(spinlock_t* lock)
{
    if (lock != NULL) {
        __sync_lock_release(&lock->locked);
    }
}

int spinlock_try_acquire(spinlock_t* lock)
{
    if (lock == NULL) {
        return 0;
    }

    return !__sync_lock_test_and_set(&lock->locked, 1);
}

// === SCHEDULER LOCK ===

void lock_scheduler(void)
{
    spinlock_acquire(&scheduler_lock);
}

void unlock_scheduler(void)
{
    spinlock_release(&scheduler_lock);
}
