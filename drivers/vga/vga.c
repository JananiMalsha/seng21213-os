#include "vga.h"

#define COM1 0x3F8

static int cursor_row = 0;
static int cursor_col = 0;
static uint8_t cur_attr = 0;

static inline void outb_vga(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb_vga(uint16_t port) {
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static void serial_putc(char c) {
    int timeout = 10000;
    while ((inb_vga(COM1 + 5) & 0x20) == 0 && --timeout > 0);
    outb_vga(COM1, c);
}

static void update_hw_cursor(void) {
    uint16_t pos = (uint16_t)(cursor_row * VGA_COLS + cursor_col);
    outb_vga(0x3D4, 0x0F);
    outb_vga(0x3D5, (uint8_t)(pos & 0xFF));
    outb_vga(0x3D4, 0x0E);
    outb_vga(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static inline void vga_write_cell(int row, int col, char c, uint8_t attr) {
    volatile uint16_t *cell = VGA_ADDR + row * VGA_COLS + col;
    *cell = (uint16_t)((attr << 8) | (uint8_t)c);
}

static void scroll_up(void) {
    volatile uint16_t *vga = VGA_ADDR;
    for (int r = 0; r < VGA_ROWS - 1; r++) {
        for (int c = 0; c < VGA_COLS; c++) {
            vga[r * VGA_COLS + c] = vga[(r + 1) * VGA_COLS + c];
        }
    }
    uint16_t blank = (uint16_t)((cur_attr << 8) | ' ');
    for (int c = 0; c < VGA_COLS; c++) {
        vga[(VGA_ROWS - 1) * VGA_COLS + c] = blank;
    }
    cursor_row = VGA_ROWS - 1;
}

void vga_init(void) {
    cur_attr = VGA_ATTR(VGA_LIGHT_GREY, VGA_BLACK);
    vga_clear(VGA_BLACK);
}

void vga_clear(vga_color_t bg) {
    cur_attr = VGA_ATTR(VGA_LIGHT_GREY, bg);
    uint16_t blank = (uint16_t)((cur_attr << 8) | ' ');
    volatile uint16_t *vga = VGA_ADDR;
    for (int i = 0; i < VGA_ROWS * VGA_COLS; i++) vga[i] = blank;
    cursor_row = 0;
    cursor_col = 0;
    update_hw_cursor();
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    cur_attr = VGA_ATTR(fg, bg);
}

void vga_putchar(char c) {
    if (c == '\n') serial_putc('\r');
    serial_putc(c);

    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
    } else if (c == '\r') {
        cursor_col = 0;
    } else if (c == '\b') {
        if (cursor_col > 0) cursor_col--;
        vga_write_cell(cursor_row, cursor_col, ' ', cur_attr);
    } else {
        vga_write_cell(cursor_row, cursor_col, c, cur_attr);
        cursor_col++;
        if (cursor_col >= VGA_COLS) { cursor_col = 0; cursor_row++; }
    }
    if (cursor_row >= VGA_ROWS) scroll_up();
    update_hw_cursor();
}

void vga_puts(const char *str) {
    if (!str) return;
    while (*str) vga_putchar(*str++);
}

void vga_puts_color(const char *str, vga_color_t fg, vga_color_t bg) {
    uint8_t saved = cur_attr;
    vga_set_color(fg, bg);
    vga_puts(str);
    cur_attr = saved;
}

static void print_uint(uint32_t n, int base) {
    char buf[32];
    int i = 0;
    if (n == 0) { vga_putchar('0'); return; }
    while (n > 0) {
        int r = n % base;
        buf[i++] = (r < 10) ? ('0' + r) : ('a' + r - 10);
        n /= base;
    }
    while (i > 0) vga_putchar(buf[--i]);
}

void vga_printf(const char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    while (*fmt) {
        if (*fmt != '%') { vga_putchar(*fmt++); continue; }
        fmt++;
        switch (*fmt) {
            case 's': vga_puts(__builtin_va_arg(args, const char *)); break;
            case 'c': vga_putchar((char)__builtin_va_arg(args, int)); break;
            case 'd': {
                int v = __builtin_va_arg(args, int);
                if (v < 0) { vga_putchar('-'); v = -v; }
                print_uint((uint32_t)v, 10);
                break;
            }
            case 'u': print_uint(__builtin_va_arg(args, uint32_t), 10); break;
            case 'x': print_uint(__builtin_va_arg(args, uint32_t), 16); break;
            case '%': vga_putchar('%'); break;
            default:  vga_putchar(*fmt); break;
        }
        fmt++;
    }
    __builtin_va_end(args);
}
