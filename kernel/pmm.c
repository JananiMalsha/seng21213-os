#include "pmm.h"
#include "string.h"

static uint32_t bitmap[BITMAP_SIZE];
static uint32_t total_frames = 0;
static uint32_t free_frames = 0;

static inline void bitmap_set(uint32_t f)   { bitmap[f/32] |=  (1u << (f%32)); }
static inline void bitmap_clear(uint32_t f) { bitmap[f/32] &= ~(1u << (f%32)); }
static inline int  bitmap_test(uint32_t f)  { return (bitmap[f/32] >> (f%32)) & 1; }

void pmm_init(void) {
    memset(bitmap, 0xFF, sizeof(bitmap));
    total_frames = (MAX_MEM_MB * 1024 * 1024) / FRAME_SIZE;
    free_frames = 0;

    uint16_t count = *(uint16_t *)0x8000;
    struct e820_entry *map = (struct e820_entry *)0x8004;

    if (count == 0) {
        for (uint32_t a = KERNEL_RESERVED; a < (MAX_MEM_MB * 1024 * 1024); a += FRAME_SIZE) {
            bitmap_clear(a / FRAME_SIZE);
            free_frames++;
        }
        return;
    }

    for (int i = 0; i < count; i++) {
        if (map[i].type == 1) {
            uint32_t start = (uint32_t)map[i].base;
            uint32_t len   = (uint32_t)map[i].length;
            uint32_t first = (start < KERNEL_RESERVED) ? KERNEL_RESERVED : start;
            for (uint32_t a = first; a < start + len && (a / FRAME_SIZE) < total_frames; a += FRAME_SIZE) {
                bitmap_clear(a / FRAME_SIZE);
                free_frames++;
            }
        }
    }
}

uint32_t pmm_alloc_frame(void) {
    for (uint32_t i = 0; i < total_frames; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            free_frames--;
            return i * FRAME_SIZE;
        }
    }
    return 0;
}

void pmm_free_frame(uint32_t phys) {
    uint32_t frame = phys / FRAME_SIZE;
    if (frame < total_frames && bitmap_test(frame)) {
        bitmap_clear(frame);
        free_frames++;
    }
}

uint32_t pmm_free_frames(void)  { return free_frames; }
uint32_t pmm_total_frames(void) { return total_frames; }
