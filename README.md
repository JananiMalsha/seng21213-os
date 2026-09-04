# SENG 21213: Computer Architecture & Operating Systems
## Bare-Metal x86 Operating System Kernel

- **Repository Visibility**: Private
- **Target Architecture**: x86-32 (i686), Protected Mode
- **Reviewer / Collaborator**: `tiroshanm`

---

## 1. Milestone & Lecture Overview

| Milestone | Tag | Lecture Reference | Implemented Features |
|---|---|---|---|
| **Stage 0** | `v0.1-stage0` | L07 §4, L08 §2–3 | 512-byte MBR bootloader, GDT, 32-bit Protected Mode transition, VGA driver at `0xB8000`, PS/2 keyboard driver, interactive shell. |
| **Stage 1** | `v0.2-stage1` | L09 §2–4 | 256-entry IDT, 8259 PIC remapping, i8253 PIT timer at 100 Hz, 16-entry PCB table, NASM context switch, Round-Robin scheduler (`ps`, `kill`). |
| **Stage 2** | `v0.3-stage2` | L10 §4–5 | Kernel threads, blocking mutex with atomic `XCHG`, counting semaphore, concurrent worker demo (`race`). |
| **Stage 3** | `v0.4-stage3` | L11 §3, §5 | BIOS E820 memory map detection, 4 KB physical frame bitmap allocator, memory inspector (`meminfo`). |
| **Stage 4** | `v0.5-stage4` | L12 §2–4 | 1 MB RAM disk, inode file system, POSIX API (`fs_open/read/write/close/unlink`), shell commands (`ls`, `touch`, `write`, `cat`, `rm`). |

---

## 2. Bonus Extensions & Test Instructions

### Extension 1: Dual-Channel I/O (VGA + Serial COM1)
Kernel output mirrors to both the VGA buffer (`0xB8000`) and Serial COM1 (`0x3F8`), enabling headless terminal testing.
```bash
make run
```
Output appears both in the QEMU window and directly in your terminal.

### Extension 2: Hardware-Atomic XCHG Mutex
Mutual exclusion implemented with the x86 `xchgl` instruction (atomic test-and-set).
```text
kernel> race
kernel> ps
```

### Extension 3: BIOS E820 Memory Auto-Detection
Real-mode bootloader queries BIOS INT 0x15 (EAX=0xE820) to build the physical memory map before switching to protected mode.
```text
kernel> meminfo
```

### Extension 4: POSIX-style Inode File System
Superblock, inode table, direct block pointers, POSIX-style API.
```text
kernel> touch bonus.txt
kernel> write bonus.txt Testing SENG21213 Inode System
kernel> cat bonus.txt
kernel> ls
kernel> rm bonus.txt
kernel> ls
```

---

## 3. Build and Run

```bash
make clean
make run
```
