org 0x7c00

MOV AX, 3
INT 10h


inizio:
	MOV CX, 0
	MOV SI, buffer
	CALL free_buffer
	MOV SI, msg1
	CALL print_string
	MOV DI, buffer

loop_input:
	CALL char_input
	CMP AL, 13
	JE input_done
	CMP AL, 8
	JE backspace
	CMP AL, ' '
	JE loop_input
	CALL get_cursor
	CMP DL, 79
	JE loop_input
	MOV [DI], AL
	INC DI
	CALL char_output
	JMP loop_input

input_done:
	; MOV BYTE [DI], 0
	MOV SI, newline
	CALL print_string
	JMP check_strings_length

check_strings_length:
	MOV SI, buffer
	MOV CX, 0
	CALL get_string_length
	PUSH CX

	MOV SI, password
	MOV CX, 0
	CALL get_string_length
	MOV AL, CL
	POP CX
	CMP AL, CL
	JNE wrong
	MOV SI, buffer
	MOV DI, password
	JMP check_strings

check_strings:
	MOV DL, [SI]
	MOV CL, [DI]
	CMP DL, 0
	JE right
	CMP DL, CL
	JNE wrong
	INC SI
	INC DI
	JMP check_strings

wrong:
	MOV SI, msg3
	CALL print_string
	MOV SI, newline
	CALL print_string
	JMP inizio

right:
	MOV SI, msg2
	CALL print_string
	JMP end

get_string_length:
	MOV AL, [SI]
	CMP AL, 0
	JE return
	INC SI
	INC CL
	JMP get_string_length


end:
	JMP $

return:
	ret

backspace:
	CALL get_cursor
	CMP DL, 19
	JE loop_input
	MOV AL, 8
	CALL char_output
	MOV AL, ' '
	CALL char_output
	CALL dec_cursor
	DEC DI
	JMP loop_input

dec_cursor:
	CALL get_cursor
	DEC DL
	MOV AH, 2
	INT 10h
	ret

get_cursor:
	MOV AH, 3
	INT 10h
	ret

print_string:
	MOV AL, [SI]
	CMP AL, 0
	JE return
	CALL char_output
	INC SI
	JMP print_string

char_input:
	MOV AH, 0
	INT 16h
	ret

char_output:
	MOV AH, 0x0e
	INT 10h
	ret

free_buffer:
	CMP CL, buffer_size
	JE return
	MOV BYTE [SI],0
	INC SI
	INC CL
	JMP free_buffer




msg1 db "Inserire password: ",0
msg2 db "La password e' giusta",0
msg3 db "La password e' sbagliata",0
buffer_size equ 64
buffer db buffer_size dup(?)
newline db 10,13,0
password db "ciao",0



times 510-($-$$) db 0
dw 0xAA55