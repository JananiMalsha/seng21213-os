#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include "process.h"

typedef struct {
    volatile int count;
    int          waiters[MAX_PROCS];
    int          nwaiters;
} semaphore_t;

void sem_init(semaphore_t *s, int initial);
void sem_wait(semaphore_t *s);
void sem_signal(semaphore_t *s);

#endif
