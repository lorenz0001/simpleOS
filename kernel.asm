bits 32
section .text
    align 4
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0x00)

global start
extern kmain

start:
    cli
    mov esp, stack_top
    call kmain
.hang:
    hlt
    jmp .hang

section .bss
    resb 8192
stack_top:
