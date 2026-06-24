; String utilities (16-bit real mode)

; Compare null-terminated strings DS:SI and ES:DI
; Returns with Zero Flag (ZF) set if equal, cleared if not equal
strcmp:
    push si
    push di
    push ax
    push bx
.loop:
    mov al, [si]
    mov bl, [di]
    cmp al, bl
    jne .not_equal
    cmp al, 0          ; If they are equal and we hit null-terminator
    je .equal
    inc si
    inc di
    jmp .loop

.not_equal:
    pop bx
    pop ax
    pop di
    pop si
    mov al, 0
    cmp al, 1          ; Clear Zero Flag (ZF = 0)
    ret

.equal:
    pop bx
    pop ax
    pop di
    pop si
    xor ax, ax         ; Set Zero Flag (ZF = 1)
    ret
