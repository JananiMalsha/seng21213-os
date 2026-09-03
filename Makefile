AS       := nasm
CC       := gcc
LD       := ld

CFLAGS   := -m32 -ffreestanding -fno-stack-protector -fno-pie -nostdlib -std=gnu99 -Wall -Wextra -O2 -I./include -I./kernel -I./drivers/vga -I./drivers/keyboard
LDFLAGS  := -m elf_i386 -nostdlib
ASFLAGS  := -f elf32

BUILD    := build
OS_IMAGE := seng21213.img

BOOT_SRC := boot/boot.asm
BOOT_BIN := $(BUILD)/boot.bin

ASM_SRCS := kernel/kernel_entry.asm $(wildcard boot/switch.asm)
ASM_OBJS := $(patsubst %.asm, $(BUILD)/%.o, $(ASM_SRCS))

C_SRCS   := $(wildcard kernel/*.c) $(wildcard drivers/vga/*.c) $(wildcard drivers/keyboard/*.c)
C_OBJS   := $(patsubst %.c, $(BUILD)/%.o, $(C_SRCS))

KERNEL_ELF := $(BUILD)/kernel.elf
KERNEL_BIN := $(BUILD)/kernel.bin

.PHONY: all clean run run-term debug

all: $(OS_IMAGE)

$(BUILD):
	@mkdir -p $(BUILD)/boot $(BUILD)/kernel $(BUILD)/drivers/vga $(BUILD)/drivers/keyboard

$(BOOT_BIN): $(BOOT_SRC) | $(BUILD)
	$(AS) -f bin $< -o $@

$(BUILD)/%.o: %.asm | $(BUILD)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(ASM_OBJS) $(C_OBJS)
	$(LD) $(LDFLAGS) -T linker.ld $^ -o $@

$(KERNEL_BIN): $(KERNEL_ELF)
	objcopy -O binary $< $@

$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	dd if=/dev/zero of=$(OS_IMAGE) bs=512 count=2880 2>/dev/null
	dd if=$(BOOT_BIN) of=$(OS_IMAGE) bs=512 count=1 conv=notrunc 2>/dev/null
	dd if=$(KERNEL_BIN) of=$(OS_IMAGE) bs=512 seek=1 conv=notrunc 2>/dev/null
	@echo "[SUCCESS] Build complete: $(OS_IMAGE)"

run: $(OS_IMAGE)
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE) -m 32M -serial mon:stdio

run-term: $(OS_IMAGE)
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE) -m 32M -nographic -serial mon:stdio

debug: $(OS_IMAGE)
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE) -m 32M -s -S -serial mon:stdio

clean:
	rm -rf $(BUILD) $(OS_IMAGE)
