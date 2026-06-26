org 0x7c00

inizio:
	MOV AX, 3
	INT 10h

	MOV SI, MSG1
	CALL print_string

	MOV DI, buffer
	CALL loop_input
	MOV BYTE [DI], 0

	MOV SI, new_line
	CALL print_string

	;MOV SI, buffer
	;CALL string_to_numbers

end:
	jmp $

return:
	ret

loop_input:
	CALL input_char
	CMP AL, 13
	JE return
	MOV [DI], AL
	INC DI
	CALL print_char
	JMP loop_input


print_string:
	MOV AL, [SI]
	CMP AL, 0
	JE return
	INC SI
	CALL print_char
	JMP print_string

print_char:
	MOV AH, 0x0e
	INT 10h
	ret

input_char:
	MOV AH, 0
	INT 16h
	ret

MSG1 DB "Inserire numero: ",0
buffer_size equ 64
buffer DB buffer_size dup(?)
D DB ?
U DB ?

new_line DB 10,13,0

times 510-($-$$) db 0
dw 0xAA55