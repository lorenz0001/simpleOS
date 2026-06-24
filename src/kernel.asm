[org 0x1000]
[bits 16]

start:
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
            db "   Antigravity Terminal OS v0.3-Alpha   ", 0x0d, 0x0a
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
CMD_INFO    db "info", 0
CMD_EXIT    db "exit", 0

MSG_BOOT_HELP db "Available Bootloader commands (16-bit BIOS):", 0x0d, 0x0a
              db "  help   - Show this help message", 0x0d, 0x0a
              db "  hello  - Print a welcome greeting", 0x0d, 0x0a
              db "  kernel - Switch to 32-bit Protected Mode", 0x0d, 0x0a
              db "  clear  - Clear the screen", 0x0d, 0x0a
              db "  reboot - Restart the system", 0x0d, 0x0a, 0

MSG_HELLO_OP db "Hello from the custom OS kernel! We hope you have fun.", 0x0d, 0x0a, 0


; =====================================================================
; 32-bit Protected Mode Section
; =====================================================================
[bits 32]

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

    ; Flush keyboard input buffer
    call flush_keyboard_pm

    ; Clear screen
    call clear_screen_pm

    ; Print welcome message
    mov ebx, MSG_PM_WELCOME
    call print_string_pm

.pm_shell_loop:
    mov ebx, MSG_PM_PROMPT
    call print_string_pm

    call read_line_pm

    call process_kernel_command_pm
    jmp .pm_shell_loop

; =====================================================================
; PM Subroutine: print_string_pm
; =====================================================================
print_string_pm:
    pusha
    mov edx, 0xb8000
    mov edi, [cursor_pos]
.loop:
    mov al, [ebx]
    cmp al, 0
    je .done
    
    cmp al, 0x0a
    je .newline
    cmp al, 0x0d
    je .carriage_return
    
    mov [edx + edi], al
    mov byte [edx + edi + 1], 0x0a ; Light green text on black
    add edi, 2
    jmp .next
    
.newline:
    push eax
    push edx
    xor edx, edx
    mov eax, edi
    mov ecx, 160
    div ecx
    inc eax
    mul ecx
    mov edi, eax
    pop edx
    pop eax
    jmp .next

.carriage_return:
    push eax
    push edx
    xor edx, edx
    mov eax, edi
    mov ecx, 160
    div ecx
    mul ecx
    mov edi, eax
    pop edx
    pop eax
    
.next:
    inc ebx
    jmp .loop
    
.done:
    cmp edi, 4000
    jl .no_scroll
    call scroll_screen_pm
    mov edi, 3840
.no_scroll:
    mov [cursor_pos], edi
    call update_hardware_cursor
    popa
    ret

; =====================================================================
; PM Subroutine: scroll_screen_pm
; =====================================================================
scroll_screen_pm:
    pusha
    mov esi, 0xb8000 + 160
    mov edi, 0xb8000
    mov ecx, 1920       ; 24 lines * 80 chars
    rep movsw
    
    mov edi, 0xb8000 + 3840
    mov ecx, 80
    mov ax, 0x0a20      ; Space with light green attribute
    rep stosw
    popa
    ret

; =====================================================================
; PM Subroutine: clear_screen_pm
; =====================================================================
clear_screen_pm:
    pusha
    mov edi, 0xb8000
    mov ecx, 2000       ; 80 * 25 cells
    mov ax, 0x0a20      ; Space with light green attribute
.loop:
    mov [edi], ax
    add edi, 2
    loop .loop
    
    mov dword [cursor_pos], 0
    call update_hardware_cursor
    popa
    ret

; =====================================================================
; PM Subroutine: update_hardware_cursor
; =====================================================================
update_hardware_cursor:
    pusha
    mov eax, [cursor_pos]
    shr eax, 1
    mov ecx, eax
    
    mov dx, 0x3d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x3d5
    mov al, cl
    out dx, al
    
    mov dx, 0x3d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x3d5
    mov al, ch
    out dx, al
    popa
    ret

; =====================================================================
; PM Subroutine: read_key_pm
; =====================================================================
read_key_pm:
.poll:
    in al, 0x64
    test al, 1
    jz .poll
    in al, 0x60
    ret

; =====================================================================
; PM Subroutine: flush_keyboard_pm
; =====================================================================
flush_keyboard_pm:
    push eax
.loop:
    in al, 0x64
    test al, 1
    jz .done
    in al, 0x60
    jmp .loop
.done:
    pop eax
    ret

; =====================================================================
; PM Subroutine: scancode_to_ascii
; =====================================================================
scancode_to_ascii:
    cmp al, 0x80
    jae .release_code
    movzx ebx, al
    mov al, [scancode_map + ebx]
    ret
.release_code:
    xor al, al
    ret

; =====================================================================
; PM Subroutine: read_line_pm
; =====================================================================
read_line_pm:
    pusha
    mov edi, pm_input_buffer
    mov ecx, 0
    
.loop:
    call read_key_pm
    call scancode_to_ascii
    cmp al, 0
    je .loop
    
    cmp al, 0x0d
    je .enter
    
    cmp al, 0x08
    je .backspace
    
    cmp ecx, 63
    je .loop
    
    push eax
    mov [char_to_print], al
    mov ebx, char_to_print
    call print_string_pm
    pop eax
    
    stosb
    inc ecx
    jmp .loop
    
.backspace:
    cmp ecx, 0
    je .loop
    dec edi
    dec ecx
    
    mov edx, [cursor_pos]
    sub edx, 2
    mov [cursor_pos], edx
    
    push ebx
    mov ebx, msg_space
    call print_string_pm
    pop ebx
    
    mov edx, [cursor_pos]
    sub edx, 2
    mov [cursor_pos], edx
    call update_hardware_cursor
    jmp .loop
    
.enter:
    mov al, 0
    stosb
    
    mov ebx, msg_newline
    call print_string_pm
    popa
    ret

; =====================================================================
; PM Subroutine: process_kernel_command_pm
; =====================================================================
process_kernel_command_pm:
    pusha
    mov esi, pm_input_buffer
    cmp byte [esi], 0
    je .done

    mov esi, pm_input_buffer
    mov edi, CMD_HELP
    call strcmp_pm
    je .cmd_help

    mov esi, pm_input_buffer
    mov edi, CMD_INFO
    call strcmp_pm
    je .cmd_info

    mov esi, pm_input_buffer
    mov edi, CMD_CLEAR
    call strcmp_pm
    je .cmd_clear

    mov esi, pm_input_buffer
    mov edi, CMD_REBOOT
    call strcmp_pm
    je .cmd_reboot

    mov esi, pm_input_buffer
    mov edi, CMD_EXIT
    call strcmp_pm
    je .cmd_exit

    mov ebx, MSG_PM_UNKNOWN
    call print_string_pm
    jmp .done

.cmd_help:
    mov ebx, MSG_PM_HELP
    call print_string_pm
    jmp .done

.cmd_info:
    mov ebx, MSG_PM_INFO
    call print_string_pm
    jmp .done

.cmd_clear:
    call clear_screen_pm
    jmp .done

.cmd_reboot:
.cmd_exit:
    mov ebx, MSG_PM_REBOOTING
    call print_string_pm
    mov ecx, 0x02000000
.delay:
    dec ecx
    jnz .delay
    mov al, 0xfe
    out 0x64, al
    jmp $

.done:
    popa
    ret

; =====================================================================
; PM Subroutine: strcmp_pm
; =====================================================================
strcmp_pm:
    push esi
    push edi
    push eax
    push ebx
.loop:
    mov al, [esi]
    mov bl, [edi]
    cmp al, bl
    jne .not_equal
    cmp al, 0
    je .equal
    inc esi
    inc edi
    jmp .loop
.not_equal:
    pop ebx
    pop eax
    pop edi
    pop esi
    mov al, 0
    cmp al, 1
    ret
.equal:
    pop ebx
    pop eax
    pop edi
    pop esi
    xor eax, eax
    ret

; =====================================================================
; 32-bit Data definitions
; =====================================================================
cursor_pos dd 0
char_to_print db 0, 0
msg_space db ' ', 0
msg_newline db 0x0d, 0x0a, 0

MSG_PM_WELCOME db "=========================================", 0x0d, 0x0a
               db "  Antigravity Kernel Space (32-bit PM)   ", 0x0d, 0x0a
               db "  Direct Hardware Access - BIOS Disabled ", 0x0d, 0x0a
               db "=========================================", 0x0d, 0x0a
               db "Type 'help' for bare-metal commands.", 0x0d, 0x0a, 0x0d, 0x0a, 0

MSG_PM_PROMPT db "kernel# ", 0

MSG_PM_HELP db "Available Protected Mode commands:", 0x0d, 0x0a
            db "  help   - Show this bare-metal command list", 0x0d, 0x0a
            db "  info   - Display raw processor state information", 0x0d, 0x0a
            db "  clear  - Clear screen (direct VGA memory write)", 0x0d, 0x0a
            db "  exit   - Reboot back to 16-bit Bootloader Mode", 0x0d, 0x0a
            db "  reboot - Pulse reset line to reboot computer", 0x0d, 0x0a, 0

MSG_PM_INFO db "System State Information (32-bit PMODE):", 0x0d, 0x0a
            db "  CPU Mode: 32-bit Protected Mode", 0x0d, 0x0a
            db "  Video RAM: 0xB8000 (VGA Text Buffer)", 0x0d, 0x0a
            db "  Keyboard: direct port I/O 0x60/0x64", 0x0d, 0x0a
            db "  Interrupts: disabled (CLI)", 0x0d, 0x0a
            db "  CS Selector: 0x08 (Code segment)", 0x0d, 0x0a
            db "  DS Selector: 0x10 (Data segment)", 0x0d, 0x0a, 0

MSG_PM_UNKNOWN   db "Error: Command not found in PMODE. Type 'help'.", 0x0d, 0x0a, 0
MSG_PM_REBOOTING db "Resetting CPU via 8042 PS/2 controller...", 0x0d, 0x0a, 0

scancode_map:
    db 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x08, 0x09
    db 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0x0d, 0
    db 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`', 0, 92
    db 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    times 256-($-scancode_map) db 0

pm_input_buffer times 64 db 0
