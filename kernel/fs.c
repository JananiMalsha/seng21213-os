#include "fs.h"
#include "string.h"

static superblock_t sb;
static inode_t      inode_table[MAX_INODES];
static uint8_t      block_bitmap[TOTAL_BLOCKS / 8];

typedef struct {
    int      inode_idx;
    uint32_t offset;
    int      flags;
    bool     used;
} file_desc_t;

static file_desc_t open_files[MAX_OPEN_FILES];

static int alloc_block(void) {
    for (uint32_t i = 16; i < TOTAL_BLOCKS; i++) {
        if (!(block_bitmap[i / 8] & (1 << (i % 8)))) {
            block_bitmap[i / 8] |= (1 << (i % 8));
            return i;
        }
    }
    return -1;
}

static void free_block(int b) {
    if (b >= 16 && b < TOTAL_BLOCKS) {
        block_bitmap[b / 8] &= ~(1 << (b % 8));
    }
}

void fs_init(void) {
    ramdisk_init();
    memset(inode_table, 0, sizeof(inode_table));
    memset(block_bitmap, 0, sizeof(block_bitmap));
    memset(open_files, 0, sizeof(open_files));

    sb.magic = FS_MAGIC;
    sb.total_blocks = TOTAL_BLOCKS;
    sb.total_inodes = MAX_INODES;
    sb.free_blocks  = TOTAL_BLOCKS - 16;
    sb.free_inodes  = MAX_INODES;
    sb.inode_table_block = 2;
    sb.data_start_block  = 16;

    for (int i = 0; i < 16; i++) {
        block_bitmap[i / 8] |= (1 << (i % 8));
    }
}

int fs_open(const char *name, int flags) {
    int ino = -1;
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_table[i].type == 1 && strcmp(inode_table[i].name, name) == 0) {
            ino = i;
            break;
        }
    }

    if (ino == -1) {
        if (!(flags & O_CREAT)) return -1;
        for (int i = 0; i < MAX_INODES; i++) {
            if (inode_table[i].type == 0) {
                ino = i;
                inode_table[i].type = 1;
                inode_table[i].size = 0;
                inode_table[i].block_count = 0;
                strncpy(inode_table[i].name, name, MAX_NAME_LEN - 1);
                break;
            }
        }
        if (ino == -1) return -1;
    }

    if (flags & O_TRUNC) {
        for (uint32_t b = 0; b < inode_table[ino].block_count; b++) {
            free_block(inode_table[ino].blocks[b]);
        }
        inode_table[ino].size = 0;
        inode_table[ino].block_count = 0;
    }

    for (int f = 0; f < MAX_OPEN_FILES; f++) {
        if (!open_files[f].used) {
            open_files[f].used = true;
            open_files[f].inode_idx = ino;
            open_files[f].flags = flags;
            open_files[f].offset = 0;
            return f;
        }
    }
    return -1;
}

int fs_read(int fd, void *buf, int n) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_files[fd].used) return -1;
    inode_t *ino = &inode_table[open_files[fd].inode_idx];
    if (open_files[fd].offset >= ino->size) return 0;

    int bytes_to_read = n;
    if (open_files[fd].offset + bytes_to_read > ino->size) {
        bytes_to_read = ino->size - open_files[fd].offset;
    }

    uint8_t *out = (uint8_t *)buf;
    int read_total = 0;
    uint8_t temp[BLOCK_SIZE];

    while (read_total < bytes_to_read) {
        uint32_t b_idx = open_files[fd].offset / BLOCK_SIZE;
        uint32_t b_off = open_files[fd].offset % BLOCK_SIZE;
        uint32_t chunk = BLOCK_SIZE - b_off;
        if (chunk > (uint32_t)(bytes_to_read - read_total)) {
            chunk = bytes_to_read - read_total;
        }

        ramdisk_read_block(ino->blocks[b_idx], temp);
        memcpy(out + read_total, temp + b_off, chunk);

        read_total += chunk;
        open_files[fd].offset += chunk;
    }
    return read_total;
}

int fs_write(int fd, const void *buf, int n) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_files[fd].used) return -1;
    inode_t *ino = &inode_table[open_files[fd].inode_idx];

    const uint8_t *in = (const uint8_t *)buf;
    int written_total = 0;
    uint8_t temp[BLOCK_SIZE];

    while (written_total < n) {
        uint32_t b_idx = open_files[fd].offset / BLOCK_SIZE;
        uint32_t b_off = open_files[fd].offset % BLOCK_SIZE;
        if (b_idx >= INODE_DIRECT) break;

        if (b_idx >= ino->block_count) {
            int new_b = alloc_block();
            if (new_b == -1) break;
            ino->blocks[ino->block_count++] = new_b;
            memset(temp, 0, BLOCK_SIZE);
        } else {
            ramdisk_read_block(ino->blocks[b_idx], temp);
        }

        uint32_t chunk = BLOCK_SIZE - b_off;
        if (chunk > (uint32_t)(n - written_total)) {
            chunk = n - written_total;
        }

        memcpy(temp + b_off, in + written_total, chunk);
        ramdisk_write_block(ino->blocks[b_idx], temp);

        written_total += chunk;
        open_files[fd].offset += chunk;
        if (open_files[fd].offset > ino->size) ino->size = open_files[fd].offset;
    }
    return written_total;
}

void fs_close(int fd) {
    if (fd >= 0 && fd < MAX_OPEN_FILES) open_files[fd].used = false;
}

int fs_unlink(const char *name) {
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_table[i].type == 1 && strcmp(inode_table[i].name, name) == 0) {
            for (uint32_t b = 0; b < inode_table[i].block_count; b++) {
                free_block(inode_table[i].blocks[b]);
            }
            inode_table[i].type = 0;
            inode_table[i].size = 0;
            inode_table[i].block_count = 0;
            return 0;
        }
    }
    return -1;
}

int fs_ls(inode_t *out, int max) {
    int count = 0;
    for (int i = 0; i < MAX_INODES && count < max; i++) {
        if (inode_table[i].type == 1) {
            memcpy(&out[count++], &inode_table[i], sizeof(inode_t));
        }
    }
    return count;
}
