[bits 32]

global port_byte_in
global port_byte_out

; Read a byte from a hardware port
; C signature: unsigned char port_byte_in(unsigned short port);
port_byte_in:
    push ebp
    mov ebp, esp
    
    mov dx, [ebp + 8]   ; First argument: port number (16-bit)
    in al, dx           ; Read byte into AL
    movzx eax, al       ; Zero-extend AL into EAX (C return value is in EAX)
    
    pop ebp
    ret

; Write a byte to a hardware port
; C signature: void port_byte_out(unsigned short port, unsigned char data);
port_byte_out:
    push ebp
    mov ebp, esp
    
    mov dx, [ebp + 8]   ; First argument: port number (16-bit)
    mov al, [ebp + 12]  ; Second argument: data byte (8-bit)
    out dx, al          ; Write AL to port DX
    
    pop ebp
    ret
