#include "keyboard.h"
#include "../vga/vga.h"

#define KB_DATA_PORT   0x60
#define KB_STATUS_PORT 0x64
#define KB_STATUS_OBF  0x01

#define COM1 0x3F8

static inline void outb_local(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb_local(uint16_t port) {
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static const char sc_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, '\\','z','x','c','v','b','n','m',',','.','/',
    0, '*', 0, ' ', 0
};

static const char sc_ascii_shift[128] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0, 'A','S','D','F','G','H','J','K','L',':','"','~',
    0, '|','Z','X','C','V','B','N','M','<','>','?',
    0, '*', 0, ' ', 0
};

static bool shift_held = false;

static void serial_init(void) {
    outb_local(COM1 + 1, 0x00);
    outb_local(COM1 + 3, 0x80);
    outb_local(COM1 + 0, 0x03);
    outb_local(COM1 + 1, 0x00);
    outb_local(COM1 + 3, 0x03);
    outb_local(COM1 + 2, 0xC7);
    outb_local(COM1 + 4, 0x0B);
}

void kb_init(void) {
    serial_init();
    while (inb_local(KB_STATUS_PORT) & KB_STATUS_OBF) {
        inb_local(KB_DATA_PORT);
    }
}

static int serial_received(void) {
    return inb_local(COM1 + 5) & 1;
}

static char serial_read(void) {
    return (char)inb_local(COM1);
}

char kb_getchar(void) {
    while (1) {
        // 1. Check Serial Port (terminal input)
        if (serial_received()) {
            char c = serial_read();
            if (c == '\r') c = '\n';
            if (c == 127)  c = '\b';
            return c;
        }

        // 2. Check PS/2 Port (QEMU graphical window)
        if (inb_local(KB_STATUS_PORT) & KB_STATUS_OBF) {
            uint8_t sc = inb_local(KB_DATA_PORT);
            if (sc & 0x80) {
                uint8_t rel = sc & 0x7F;
                if (rel == 0x2A || rel == 0x36) shift_held = false;
                continue;
            }
            if (sc == 0x2A || sc == 0x36) { shift_held = true; continue; }
            char c = shift_held ? sc_ascii_shift[sc] : sc_ascii[sc];
            if (c) return c;
        }
    }
}

int kb_readline(char *buf, int len) {
    int i = 0;
    while (i < len - 1) {
        char c = kb_getchar();
        if (c == '\n' || c == '\r') {
            vga_putchar('\n');
            break;
        }
        if (c == '\b') {
            if (i > 0) {
                i--;
                vga_putchar('\b');
            }
            continue;
        }
        buf[i++] = c;
        vga_putchar(c);
    }
    buf[i] = '\0';
    return i;
}
