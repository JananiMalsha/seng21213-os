#include "idt.h"
#include "string.h"

static struct idt_entry idt[256];
static struct idt_ptr   idtp;

extern void isr32_timer(void);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;
    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait();
    outb(0xA1, 0x28); io_wait();
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();
    outb(0x21, 0x00); io_wait();
    outb(0xA1, 0xFF); io_wait();

    idt_set_gate(32, (uint32_t)isr32_timer, 0x08, 0x8E);

    __asm__ volatile("lidt %0" : : "m"(idtp));
}

void pit_init(void) {
    uint16_t divisor = 11931; // 100 Hz
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}
