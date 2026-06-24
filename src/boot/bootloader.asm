[org 0x7c00]

; Set up segments and stack
xor ax, ax
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00         ; Stack grows down from 0x7C00

; Save boot drive passed by BIOS in DL
mov [BOOT_DRIVE], dl

; Print bootloader status
mov si, MSG_BOOT
call print_string

; Load kernel from disk
mov bx, KERNEL_OFFSET  ; Buffer offset (ES:BX = 0x0000:0x1000)
mov dh, 40             ; Read 40 sectors (plenty of room for C kernel expansion)
mov dl, [BOOT_DRIVE]   ; Use saved boot drive
call disk_load

; Jump to kernel entry point
jmp KERNEL_OFFSET

; Include utilities
%include "print.asm"
%include "disk.asm"

; Data definitions
BOOT_DRIVE db 0
KERNEL_OFFSET equ 0x1000
MSG_BOOT db "Bootloader: Loading kernel from disk...", 0x0d, 0x0a, 0

; Pad to 510 bytes and append magic number
times 510-($-$$) db 0
dw 0xaa55
