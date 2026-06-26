mov ax, 3
int 10h

mov ah, 14

mov al, 65

loop:
int 10h
inc al
cmp al, 90
jle loop

mov al, 97

loop2:
int 10h
inc al
cmp al, 122
jle loop2

mov al, 48

loop3:
int 10h
inc al
cmp al, 57
jle loop3






jmp $
times 510-($-$$) db 0
db 0x55, 0xaa
