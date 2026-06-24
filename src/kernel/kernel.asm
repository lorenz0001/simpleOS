section .text
global _start
_start:
[bits 16]

    ; Print the welcome banner
    mov si, MSG_WELCOME
    call print_string

.shell_loop:
    ; Bootloader mode prompt
    mov si, MSG_BOOT_PROMPT
    call print_string

    ; Read input line from keyboard
    call read_line

    ; Process input
    call process_boot_command
    jmp .shell_loop

; =====================================================================
; Subroutine: process_boot_command
; =====================================================================
process_boot_command:
    pusha

    ; Check if command is empty
    mov si, input_buffer
    cmp byte [si], 0
    je .done

    ; Compare with 'help'
    mov si, input_buffer
    mov di, CMD_HELP
    call strcmp
    je .cmd_help

    ; Compare with 'hello'
    mov si, input_buffer
    mov di, CMD_HELLO
    call strcmp
    je .cmd_hello

    ; Compare with 'kernel'
    mov si, input_buffer
    mov di, CMD_KERNEL
    call strcmp
    je .cmd_kernel

    ; Compare with 'clear'
    mov si, input_buffer
    mov di, CMD_CLEAR
    call strcmp
    je .cmd_clear

    ; Compare with 'reboot'
    mov si, input_buffer
    mov di, CMD_REBOOT
    call strcmp
    je .cmd_reboot

    ; Unknown command
    mov si, MSG_UNKNOWN
    call print_string
    jmp .done

.cmd_help:
    mov si, MSG_BOOT_HELP
    call print_string
    jmp .done

.cmd_hello:
    mov si, MSG_HELLO_OP
    call print_string
    jmp .done

.cmd_kernel:
    mov si, MSG_ENTER_KERNEL
    call print_string
    
    ; Switch to 32-bit Protected Mode
    cli                     ; 1. Disable interrupts
    lgdt [gdt_descriptor]   ; 2. Load GDT

    mov eax, cr0
    or eax, 0x1             ; 3. Set PE bit in CR0
    mov cr0, eax

    jmp CODE_SEG:init_pm    ; 4. Far jump to clear pipeline and load CS

.cmd_clear:
    call clear_screen
    jmp .done

.cmd_reboot:
    ; Warm reboot
    mov ax, 0x0040
    mov ds, ax
    mov word [0x0072], 0x1234
    jmp 0xFFFF:0000

.done:
    popa
    ret

; =====================================================================
; Subroutine: clear_screen
; =====================================================================
clear_screen:
    pusha
    
    ; Scroll/clear screen
    mov ah, 0x06       ; Scroll up function
    mov al, 0          ; Clear entire screen
    mov bh, 0x07       ; Attribute: Light gray text on black background
    mov cx, 0          ; Row 0, Col 0 (upper-left)
    mov dx, 0x184f     ; Row 24, Col 79 (lower-right)
    int 0x10

    ; Set cursor position to (0,0)
    mov ah, 0x02       ; Set cursor position function
    mov bh, 0          ; Page 0
    mov dh, 0          ; Row 0
    mov dl, 0          ; Col 0
    int 0x10

    popa
    ret

; Include 16-bit dependencies
%include "print.asm"
%include "keyboard.asm"
%include "string.asm"

; =====================================================================
; GDT Definitions (Data)
; =====================================================================
gdt_start:
    dd 0x0              ; Null descriptor
    dd 0x0

gdt_code:
    dw 0xffff           ; Limit (0-15)
    dw 0x0              ; Base (0-15)
    db 0x0              ; Base (16-23)
    db 10011010b        ; Flags: Present, Ring 0, Code, Executable, Readable
    db 11001111b        ; Granularity (4KB), 32-bit PMODE, Limit (16-19)
    db 0x0              ; Base (24-31)

gdt_data:
    dw 0xffff           ; Limit (0-15)
    dw 0x0              ; Base (0-15)
    db 0x0              ; Base (16-23)
    db 10010010b        ; Flags: Present, Ring 0, Data, Writable
    db 11001111b        ; Granularity (4KB), 32-bit PMODE, Limit (16-19)
    db 0x0              ; Base (24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; 16-bit Data
MSG_WELCOME db "=========================================", 0x0d, 0x0a
            db "   Antigravity Terminal OS v0.4-Alpha   ", 0x0d, 0x0a
            db "=========================================", 0x0d, 0x0a
            db "Type 'help' to view the list of commands.", 0x0d, 0x0a, 0x0d, 0x0a, 0

MSG_BOOT_PROMPT   db "bootloader> ", 0
MSG_ENTER_KERNEL  db "Entering 32-bit Protected Mode... BIOS disabled.", 0x0d, 0x0a, 0
MSG_UNKNOWN       db "Error: Command not found. Type 'help' for assistance.", 0x0d, 0x0a, 0

CMD_HELP    db "help", 0
CMD_HELLO   db "hello", 0
CMD_CLEAR   db "clear", 0
CMD_REBOOT  db "reboot", 0
CMD_KERNEL  db "kernel", 0

MSG_BOOT_HELP db "Available Bootloader commands (16-bit BIOS):", 0x0d, 0x0a
              db "  help   - Show this help message", 0x0d, 0x0a
              db "  hello  - Print a welcome greeting", 0x0d, 0x0a
              db "  kernel - Switch to 32-bit PMODE C-Kernel", 0x0d, 0x0a
              db "  clear  - Clear the screen", 0x0d, 0x0a
              db "  reboot - Restart the system", 0x0d, 0x0a, 0

MSG_HELLO_OP db "Hello from the custom OS kernel! We hope you have fun.", 0x0d, 0x0a, 0


; =====================================================================
; 32-bit Protected Mode Section
; =====================================================================
[bits 32]
extern kernel_main

init_pm:
    ; Update segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Setup stack
    mov ebp, 0x90000
    mov esp, ebp

    ; Call our C kernel_main
    call kernel_main

    ; Loop forever if kernel returns
    jmp $
