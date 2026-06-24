; BIOS printing utility (16-bit real mode)

; Print a null-terminated string pointed to by SI
print_string:
    pusha
    mov ah, 0x0e      ; BIOS teletype function
.loop:
    lodsb             ; Load AL with byte at DS:SI, increment SI
    cmp al, 0         ; Check if null terminator
    je .done
    int 0x10          ; Call BIOS interrupt to print char in AL
    jmp .loop
.done:
    popa
    ret

; Print a newline character (Carriage Return + Line Feed)
print_nl:
    pusha
    mov ah, 0x0e
    mov al, 0x0d      ; Carriage return
    int 0x10
    mov al, 0x0a      ; Line feed
    int 0x10
    popa
    ret
