org 0x7c00

mov ax, 3h
int 10h

check_strings_length:
	MOV SI, string1
	MOV CX, 0
	CALL get_string_length
	PUSH CX

	MOV SI, string2
	MOV CX, 0
	CALL get_string_length
	MOV AL, CL
	POP CX
	CMP AL, CL
	JNE wrong
	MOV SI, string1
	MOV DI, string2
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
	MOV SI, MSG2
	CALL print_string
	JMP end

right:
	MOV SI, MSG1
	CALL print_string
	JMP end

return:
	ret

print_string:
	MOV AL, [SI]
	CMP AL, 0
	JE return
	CALL print_char
	INC SI
	JMP print_string
	ret

print_char:
	MOV AH, 0x0e
	INT 10h
	ret

get_string_length:
	MOV AL, [SI]
	CMP AL, 0
	JE return
	INC SI
	INC CL
	JMP get_string_length



end:
	jmp $


string1 db "odio",0
string2 db "odio",0
MSG1 db "giusto",0
MSG2 db "sbagliato",0

times 510-($-$$) db 0
dw 0xAA55