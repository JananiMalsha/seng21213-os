#include "semaphore.h"
#include "scheduler.h"

void sem_init(semaphore_t *s, int initial) {
    s->count = initial;
    s->nwaiters = 0;
}

void sem_wait(semaphore_t *s) {
    __asm__ volatile("cli");
    s->count--;
    if (s->count < 0) {
        s->waiters[s->nwaiters++] = current_proc;
        proc_table[current_proc].state = PROC_BLOCKED;
        __asm__ volatile("sti");
        scheduler_yield();
        return;
    }
    __asm__ volatile("sti");
}

void sem_signal(semaphore_t *s) {
    __asm__ volatile("cli");
    s->count++;
    if (s->count <= 0 && s->nwaiters > 0) {
        int pid = s->waiters[--s->nwaiters];
        proc_table[pid].state = PROC_READY;
    }
    __asm__ volatile("sti");
}
