#include "ramdisk.h"
#include "string.h"

static uint8_t ramdisk_storage[RAMDISK_SIZE];

void ramdisk_init(void) {
    memset(ramdisk_storage, 0, RAMDISK_SIZE);
}

int ramdisk_read_block(uint32_t block_num, void *buf) {
    if (block_num >= TOTAL_BLOCKS) return -1;
    memcpy(buf, &ramdisk_storage[block_num * BLOCK_SIZE], BLOCK_SIZE);
    return 0;
}

int ramdisk_write_block(uint32_t block_num, const void *buf) {
    if (block_num >= TOTAL_BLOCKS) return -1;
    memcpy(&ramdisk_storage[block_num * BLOCK_SIZE], buf, BLOCK_SIZE);
    return 0;
}
