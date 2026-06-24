; BIOS disk read utility (16-bit real mode)

; Load DH sectors from drive DL into ES:BX
disk_load:
    pusha
    push dx            ; Save DX (contains boot drive and requested sectors)

    mov ah, 0x02       ; BIOS read sector function
    mov al, dh         ; Number of sectors to read
    mov ch, 0x00       ; Cylinder 0
    mov dh, 0x00       ; Head 0
    mov cl, 0x02       ; Start reading from sector 2 (bootloader is sector 1)
    ; DL contains the boot drive number passed by the BIOS

    int 0x13           ; BIOS disk interrupt
    jc disk_error      ; Jump if Carry Flag (CF) is set (indicates disk error)

    pop dx             ; Restore original DX
    cmp al, dh         ; Compare sectors read (AL) with sectors requested (DH)
    jne disk_error     ; Jump if not equal
    popa
    ret

disk_error:
    mov si, MSG_DISK_ERROR
    call print_string
    jmp $              ; Infinite loop on disk error

MSG_DISK_ERROR db "Error reading disk!", 0x0d, 0x0a, 0
