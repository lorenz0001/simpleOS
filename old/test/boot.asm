cpu 8086
org 0x7c00

mov ax, 3h
int 10h

mov bx, 0
mov ah, 0x0e

mov si, string
call print_string

call get_cursor
inc dl
mov byte [cursor_limit], dl



loop_input:
	call get_cursor
	call input_char
	cmp al, 13
	je end
	cmp al, 8
	je call_backspace
	cmp dl, len_max
	jnb nothing
	call call_print_char
	jmp loop_input

nothing:
	jmp loop_input

end:
	jmp $

call_print_char:
	call print_char
	jmp loop_input

call_backspace:
	call get_cursor
	cmp dl, [cursor_limit]
	jc loop_input
	call backspace
	jmp loop_input

backspace:
	call dec_cursor
	mov al, ' '
	call print_char
	call dec_cursor
	jmp return

get_cursor:
	mov ah, 3
	int 10h
	jmp return

dec_cursor:
	call get_cursor
	dec dl
	mov ah, 2
	int 10h
	jmp return

print_string:
    mov bx, 0 
print_string_loop:
    mov al, [si+bx]
    cmp al, 0
    je return
    mov ah, 0x0e  
    int 10h 
    inc bx
    jmp print_string_loop

return:
    ret

print_char:
	cmp al, 0x08
	je return
	mov ah, 0x0e
	int 10h
	jmp return

input_char:
	mov ah, 0
	int 16h
	jmp return

string db "Inserire password: ",0
string2 db 0
len_max equ 35
capo db 10,13,0
cursor_limit db 0

times 510-($-$$) db 0
dw 0xAA55