#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "process.h"

extern void context_switch(uint32_t *old_esp, uint32_t new_esp);
void scheduler_tick(void);
void scheduler_yield(void);

#endif
