#include "../drivers/vga/vga.h"
#include "../drivers/keyboard/keyboard.h"
#include "string.h"
#include "process.h"
#include "scheduler.h"
#include "mutex.h"
#include "semaphore.h"
#include "pmm.h"
#include "fs.h"

static void cmd_help(int argc, char *argv[]);
static void cmd_clear(int argc, char *argv[]);
static void cmd_version(int argc, char *argv[]);
static void cmd_colour(int argc, char *argv[]);
static void cmd_echo(int argc, char *argv[]);
static void cmd_halt(int argc, char *argv[]);
static void cmd_ps(int argc, char *argv[]);
static void cmd_kill(int argc, char *argv[]);
static void cmd_race(int argc, char *argv[]);
static void cmd_meminfo(int argc, char *argv[]);
static void cmd_ls(int argc, char *argv[]);
static void cmd_touch(int argc, char *argv[]);
static void cmd_cat(int argc, char *argv[]);
static void cmd_write(int argc, char *argv[]);
static void cmd_rm(int argc, char *argv[]);

typedef struct {
    const char *name;
    const char *help;
    void (*fn)(int argc, char *argv[]);
} command_t;

static const command_t commands[] = {
    {"help",    "List all available commands", cmd_help},
    {"clear",   "Clear VGA text display",      cmd_clear},
    {"version", "Print kernel version",        cmd_version},
    {"colour",  "Change text colour (fg bg)",  cmd_colour},
    {"echo",    "Echo text to display",        cmd_echo},
    {"halt",    "Halt the CPU",                cmd_halt},
    {"ps",      "List all active processes",   cmd_ps},
    {"kill",    "Terminate process by PID",    cmd_kill},
    {"race",    "Demonstrate race condition",  cmd_race},
    {"meminfo", "Show physical memory stats",  cmd_meminfo},
    {"ls",      "List files on RAM disk",      cmd_ls},
    {"touch",   "Create empty file",           cmd_touch},
    {"cat",     "Display file contents",       cmd_cat},
    {"write",   "Write string to file",        cmd_write},
    {"rm",      "Remove file from RAM disk",   cmd_rm},
    {NULL, NULL, NULL}
};

static void cmd_help(int argc, char *argv[]) {
    (void)argc; (void)argv;
    vga_puts_color("\nAvailable SENG21213 Kernel Commands:\n", VGA_YELLOW, VGA_BLACK);
    vga_puts("  help       - List all available commands\n");
    vga_puts("  clear      - Clear VGA text display\n");
    vga_puts("  version    - Print kernel version\n");
    vga_puts("  colour     - Change text colour (fg bg)\n");
    vga_puts("  echo       - Echo text to display\n");
    vga_puts("  halt       - Halt the CPU\n");
    vga_puts("  ps         - List active processes\n");
    vga_puts("  kill       - Terminate process by PID\n");
    vga_puts("  race       - Run concurrent mutex demo\n");
    vga_puts("  meminfo    - Show physical memory stats\n");
    vga_puts("  ls         - List files on RAM disk\n");
    vga_puts("  touch      - Create empty file\n");
    vga_puts("  cat        - Display file contents\n");
    vga_puts("  write      - Write string to file\n");
    vga_puts("  rm         - Remove file from RAM disk\n\n");
}

static void cmd_clear(int argc, char *argv[]) {
    (void)argc; (void)argv;
    vga_clear(VGA_BLACK);
}

static void cmd_version(int argc, char *argv[]) {
    (void)argc; (void)argv;
    vga_puts_color("SENG21213-OS Kernel v0.5-stage4 (x86-32 Protected Mode)\n", VGA_LIGHT_CYAN, VGA_BLACK);
}

static void cmd_colour(int argc, char *argv[]) {
    if (argc >= 3) {
        int fg = argv[1][0] - '0';
        int bg = argv[2][0] - '0';
        vga_set_color(fg, bg);
    } else {
        vga_puts("Usage: colour <fg 0-15> <bg 0-15>\n");
    }
}

static void cmd_echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        vga_puts(argv[i]);
        if (i < argc - 1) vga_putchar(' ');
    }
    vga_putchar('\n');
}

static void cmd_halt(int argc, char *argv[]) {
    (void)argc; (void)argv;
    vga_puts_color("System halted.\n", VGA_LIGHT_RED, VGA_BLACK);
    __asm__ volatile("cli; hlt");
}

static void cmd_ps(int argc, char *argv[]) {
    (void)argc; (void)argv;
    const char *states[] = {"UNUSED", "READY", "RUNNING", "BLOCKED", "ZOMBIE"};
    vga_puts("PID  NAME           STATE      TICKS\n");
    vga_puts("---- -------------- ---------- -----\n");
    for (int i = 0; i < MAX_PROCS; i++) {
        if (proc_table[i].state != PROC_UNUSED) {
            vga_printf("%d    %s        %s    %u\n",
                       proc_table[i].pid, proc_table[i].name,
                       states[proc_table[i].state], proc_table[i].ticks);
        }
    }
}

static void cmd_kill(int argc, char *argv[]) {
    if (argc < 2) { vga_puts("Usage: kill <pid>\n"); return; }
    uint32_t pid = argv[1][0] - '0';
    proc_kill(pid);
    vga_printf("Process %u terminated.\n", pid);
}

static volatile int shared_counter = 0;
static mutex_t      demo_mutex;

static void race_worker(void) {
    for (int i = 0; i < 2000; i++) {
        mutex_lock(&demo_mutex);
        shared_counter++;
        mutex_unlock(&demo_mutex);
    }
}

static void cmd_race(int argc, char *argv[]) {
    (void)argc; (void)argv;
    shared_counter = 0;
    mutex_init(&demo_mutex);
    proc_create("worker1", race_worker);
    proc_create("worker2", race_worker);
    vga_puts("Spawned worker1 & worker2. Type 'ps' to inspect status.\n");
}

static void cmd_meminfo(int argc, char *argv[]) {
    (void)argc; (void)argv;
    uint32_t total = pmm_total_frames() * 4;
    uint32_t free  = pmm_free_frames() * 4;
    uint32_t used  = total - free;
    vga_printf("Memory: Total: %u KB | Used: %u KB | Free: %u KB\n", total, used, free);
}

static void cmd_ls(int argc, char *argv[]) {
    (void)argc; (void)argv;
    inode_t list[MAX_INODES];
    int n = fs_ls(list, MAX_INODES);
    vga_puts("NAME                 SIZE\n");
    vga_puts("-------------------- --------\n");
    for (int i = 0; i < n; i++) {
        vga_printf("%s                  %u B\n", list[i].name, list[i].size);
    }
    vga_printf("Total: %d file(s)\n", n);
}

static void cmd_touch(int argc, char *argv[]) {
    if (argc < 2) { vga_puts("Usage: touch <filename>\n"); return; }
    int fd = fs_open(argv[1], O_CREAT | O_WRONLY);
    if (fd >= 0) { fs_close(fd); vga_puts("Created.\n"); }
    else vga_puts("Error creating file.\n");
}

static void cmd_cat(int argc, char *argv[]) {
    if (argc < 2) { vga_puts("Usage: cat <filename>\n"); return; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { vga_puts("File not found.\n"); return; }
    char buf[256];
    int bytes;
    while ((bytes = fs_read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[bytes] = '\0';
        vga_puts(buf);
    }
    vga_putchar('\n');
    fs_close(fd);
}

static void cmd_write(int argc, char *argv[]) {
    if (argc < 3) { vga_puts("Usage: write <file> <text>\n"); return; }
    int fd = fs_open(argv[1], O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) { vga_puts("Cannot open file.\n"); return; }
    for (int i = 2; i < argc; i++) {
        fs_write(fd, argv[i], strlen(argv[i]));
        if (i < argc - 1) fs_write(fd, " ", 1);
    }
    fs_close(fd);
    vga_puts("Written successfully.\n");
}

static void cmd_rm(int argc, char *argv[]) {
    if (argc < 2) { vga_puts("Usage: rm <filename>\n"); return; }
    if (fs_unlink(argv[1]) == 0) vga_puts("Deleted.\n");
    else vga_puts("Error removing file.\n");
}

void shell_run(void) {
    char line[256];
    char *argv[16];
    vga_puts_color("\nKernel Shell ready. Type 'help' for commands.\n", VGA_LIGHT_GREEN, VGA_BLACK);

    while (1) {
        vga_puts_color("kernel> ", VGA_LIGHT_CYAN, VGA_BLACK);
        kb_readline(line, sizeof(line));

        int argc = 0;
        char *p = line;
        while (*p) {
            while (*p == ' ') *p++ = '\0';
            if (*p == '\0') break;
            if (argc < 16) argv[argc++] = p;
            while (*p && *p != ' ') p++;
        }
        if (argc == 0) continue;

        bool found = false;
        for (int i = 0; commands[i].name; i++) {
            if (strcmp(argv[0], commands[i].name) == 0) {
                commands[i].fn(argc, argv);
                found = true;
                break;
            }
        }
        if (!found) {
            vga_printf("Unknown command '%s'. Type 'help'.\n", argv[0]);
        }
    }
}
