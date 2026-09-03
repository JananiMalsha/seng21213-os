#include "mutex.h"
#include "scheduler.h"

static inline int atomic_xchg(volatile int *ptr, int val) {
    int old;
    __asm__ volatile("xchgl %0, %1" : "=r"(old), "+m"(*ptr) : "0"(val) : "memory");
    return old;
}

void mutex_init(mutex_t *m) {
    m->locked = 0;
    m->owner = -1;
    m->nwaiters = 0;
}

void mutex_lock(mutex_t *m) {
    while (atomic_xchg(&m->locked, 1) == 1) {
        m->waiters[m->nwaiters++] = current_proc;
        proc_table[current_proc].state = PROC_BLOCKED;
        scheduler_yield();
    }
    m->owner = current_proc;
}

void mutex_unlock(mutex_t *m) {
    m->owner = -1;
    m->locked = 0;
    if (m->nwaiters > 0) {
        int pid = m->waiters[--m->nwaiters];
        proc_table[pid].state = PROC_READY;
    }
}
