#ifndef PMM_H
#define PMM_H
#include "../include/types.h"

#define FRAME_SIZE      4096
#define MAX_MEM_MB      32
#define BITMAP_SIZE     (MAX_MEM_MB * 1024 * 1024 / FRAME_SIZE / 32)
#define KERNEL_RESERVED 0x600000

struct e820_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi;
} __attribute__((packed));

void     pmm_init(void);
uint32_t pmm_alloc_frame(void);
void     pmm_free_frame(uint32_t phys);
uint32_t pmm_free_frames(void);
uint32_t pmm_total_frames(void);

#endif
