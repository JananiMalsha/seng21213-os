#include "../drivers/vga/vga.h"
#include "../drivers/keyboard/keyboard.h"
#include "idt.h"
#include "serial.h"
#include "process.h"
#include "pmm.h"
#include "fs.h"

extern void shell_run(void);

void kernel_main(void) {
    vga_init();
    serial_init();
    vga_puts_color("SENG21213 OS Booting...\n", VGA_LIGHT_CYAN, VGA_BLACK);

    idt_init();
    pit_init();
    process_init();
    pmm_init();
    fs_init();
    kb_init();

    vga_puts_color("[ OK ] All Subsystems Initialized Successfully.\n", VGA_LIGHT_GREEN, VGA_BLACK);

    __asm__ volatile("sti");

    shell_run();

    while (1) { __asm__ volatile("hlt"); }
}
