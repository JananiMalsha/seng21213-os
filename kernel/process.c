#include "process.h"
#include "string.h"
#include "scheduler.h"

pcb_t proc_table[MAX_PROCS];
int   current_proc = 0;
static uint32_t next_pid = 1;

void process_init(void) {
    memset(proc_table, 0, sizeof(proc_table));
    proc_table[0].pid = 0;
    proc_table[0].state = PROC_RUNNING;
    strcpy(proc_table[0].name, "kernel_main");
    current_proc = 0;
}

static void proc_stub(void) {
    __asm__ volatile("sti");
    if (proc_table[current_proc].entry) {
        proc_table[current_proc].entry();
    }
    proc_exit();
}

pcb_t *proc_create(const char *name, void (*entry)(void)) {
    for (int i = 1; i < MAX_PROCS; i++) {
        if (proc_table[i].state == PROC_UNUSED || proc_table[i].state == PROC_ZOMBIE) {
            pcb_t *p = &proc_table[i];
            p->pid = next_pid++;
            p->state = PROC_READY;
            p->entry = entry;
            p->ticks = 0;
            strncpy(p->name, name, 31);

            uint32_t *stk = (uint32_t *)(p->stack + STACK_SIZE);
            *(--stk) = (uint32_t)proc_exit;
            *(--stk) = (uint32_t)proc_stub; // Starts process with interrupts enabled

            for (int r = 0; r < 8; r++) *(--stk) = 0;

            p->esp = (uint32_t)stk;
            return p;
        }
    }
    return NULL;
}

void proc_exit(void) {
    proc_table[current_proc].state = PROC_ZOMBIE;
    while (1) {
        scheduler_yield();
        __asm__ volatile("sti; hlt");
    }
}

void proc_kill(uint32_t pid) {
    for (int i = 1; i < MAX_PROCS; i++) {
        if (proc_table[i].pid == pid && proc_table[i].state != PROC_UNUSED) {
            proc_table[i].state = PROC_ZOMBIE;
            return;
        }
    }
}
