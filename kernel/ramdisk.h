#ifndef RAMDISK_H
#define RAMDISK_H
#include "../include/types.h"

#define RAMDISK_SIZE (1024 * 1024)
#define BLOCK_SIZE   512
#define TOTAL_BLOCKS (RAMDISK_SIZE / BLOCK_SIZE)

void ramdisk_init(void);
int  ramdisk_read_block(uint32_t block_num, void *buf);
int  ramdisk_write_block(uint32_t block_num, const void *buf);

#endif
