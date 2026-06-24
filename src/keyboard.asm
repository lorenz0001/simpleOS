; BIOS Keyboard utility (16-bit real mode)

; Read a line of input into input_buffer (max 63 characters + null terminator)
read_line:
    pusha
    mov di, input_buffer
    mov cx, 0            ; Character counter

.loop:
    ; BIOS keyboard read keystroke: AH = 00h
    mov ah, 0x00
    int 0x16             ; AL gets ASCII character

    ; Check if Enter (Carriage Return = 0x0D)
    cmp al, 0x0d
    je .enter_pressed

    ; Check if Backspace (0x08)
    cmp al, 0x08
    je .backspace_pressed

    ; Check if printable ASCII character (0x20 to 0x7E)
    cmp al, 0x20
    jl .loop             ; Ignore non-printable control characters
    cmp al, 0x7e
    jg .loop             ; Ignore non-printable control characters

    ; Check if buffer is full (max 63 chars)
    cmp cx, 63
    je .loop             ; Ignore character if buffer is full

    ; Echo character to screen
    mov ah, 0x0e
    int 0x10

    ; Store character in buffer
    stosb                ; Store AL at ES:DI, increment DI
    inc cx
    jmp .loop

.backspace_pressed:
    cmp cx, 0            ; If buffer is empty, do nothing
    je .loop

    ; Decrement buffer pointer and counter
    dec di
    dec cx

    ; Erase character on screen
    mov ah, 0x0e
    mov al, 0x08         ; Move cursor left
    int 0x10
    mov al, ' '          ; Overwrite with space
    int 0x10
    mov al, 0x08         ; Move cursor left again
    int 0x10
    jmp .loop

.enter_pressed:
    ; Null terminate the string
    mov al, 0
    stosb

    ; Print newline
    call print_nl

    popa
    ret

; Buffer for storing keyboard inputs
input_buffer times 64 db 0
