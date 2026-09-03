#include "scheduler.h"

void scheduler_tick(void) {
    proc_table[current_proc].ticks++;

    int next = (current_proc + 1) % MAX_PROCS;
    int searched = 0;
    while (searched < MAX_PROCS) {
        if (proc_table[next].state == PROC_READY) break;
        next = (next + 1) % MAX_PROCS;
        searched++;
    }

    if (searched == MAX_PROCS || next == current_proc) return;

    int old = current_proc;
    if (proc_table[old].state == PROC_RUNNING) proc_table[old].state = PROC_READY;
    proc_table[next].state = PROC_RUNNING;
    current_proc = next;

    context_switch(&proc_table[old].esp, proc_table[next].esp);
}

void scheduler_yield(void) {
    scheduler_tick();
}
