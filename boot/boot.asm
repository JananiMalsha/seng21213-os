[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl
    call detect_memory

    ; Load 64 sectors (32 KB) from disk into 0x1000:0000 (physical 0x10000)
    mov bx, 0x1000
    mov es, bx
    xor bx, bx

    mov ah, 0x02
    mov al, 64
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SEG:init_pm32

disk_error:
    jmp $

detect_memory:
    mov di, 0x8004
    xor ebx, ebx
    xor bp, bp
.e820_loop:
    mov eax, 0xE820
    mov ecx, 24
    mov edx, 0x534D4150
    int 0x15
    jc .e820_done
    inc bp
    add di, 24
    test ebx, ebx
    jnz .e820_loop
.e820_done:
    mov [0x8000], bp
    ret

[BITS 32]
init_pm32:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Place kernel stack safely at 4 MB (0x400000), well above kernel & RAM disk
    mov ebp, 0x400000
    mov esp, ebp

    call 0x10000
    hlt

boot_drive db 0

gdt_start:
gdt_null:
    dd 0x0, 0x0
gdt_code:
    dw 0xFFFF, 0x0000
    db 0x00, 10011010b, 11001111b, 0x00
gdt_data:
    dw 0xFFFF, 0x0000
    db 0x00, 10010010b, 11001111b, 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

times 510 - ($ - $$) db 0
dw 0xAA55
