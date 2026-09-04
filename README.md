# SENG21213 OS

A custom x86 (32-bit) operating system built from scratch for SENG21213, University of Kelaniya.

## Features by Stage

- **Stage 0** — 512-byte MBR bootloader (real mode → protected mode, GDT), VGA text driver, PS/2 keyboard driver, interactive shell
- **Stage 1** — Process table, i8253 PIT timer (100 Hz), round-robin scheduler, context switching
- **Stage 2** — Kernel threads, mutex (atomic XCHG), counting semaphores
- **Stage 3** — Physical memory manager: E820 memory map parsing, 4 KB frame bitmap allocator
- **Stage 4** — 1 MB RAM disk with inode-based file system (fs_open/read/write/close/unlink)

## Build & Run

```bash
make clean
make run
```

QEMU will boot the kernel and drop into the interactive shell.

## Shell Commands

| Command | Description |
|---|---|
| help | List all commands |
| clear | Clear the screen |
| echo <text> | Print text |
| version | Print kernel version |
| colour <fg> <bg> | Change text colour |
| halt | Halt the CPU |
| ps | List processes |
| kill <pid> | Terminate a process |
| race | Demonstrate mutex-protected shared counter |
| meminfo | Show physical memory usage |
| ls | List files on RAM disk |
| touch <name> | Create empty file |
| cat <name> | Print file contents |
| write <name> <text> | Write text to a file |
| rm <name> | Delete a file |

## Testing

- **Scheduler**: run `race` then `ps` — two worker processes run concurrently, incrementing a shared counter safely under a mutex.
- **Memory manager**: run `meminfo` to see total/used/free frames.
- **File system**: run `touch test.txt`, `write test.txt <text>`, `cat test.txt`, `ls`, `rm test.txt`, `ls` — verifies create/write/read/delete.

## Notes

Built and tested on Ubuntu (WSL2) with NASM, GCC (-m32, -std=gnu11), and QEMU (qemu-system-i386).
