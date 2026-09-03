[BITS 32]
GLOBAL context_switch
GLOBAL isr32_timer
EXTERN scheduler_tick

context_switch:
    pushad
    mov eax, [esp + 36]
    mov [eax], esp
    mov esp, [esp + 40]
    popad
    ret

isr32_timer:
    pushad
    ; Send EOI to PIC first so future timer interrupts work
    mov al, 0x20
    out 0x20, al
    call scheduler_tick
    popad
    iretd
