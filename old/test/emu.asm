; BIOS reads 512 bytes from cylinder: 0, head: 0, sector: 1
; of bootable floppy drive, then it loads this data into
; memory at 0000:7c00h and starts the execution from the first byte.

; note: this file is assembled into .bin file.
;       default configuration for .binf file:
;       load segment: 0000, load offset: 7c00, 
;       same values are set to CS and IP registers accordingly.

; .binf file is used by the emulator to decide at what memory
; address to load binary file and start the execution, when 
; loading address is set to 0000:7c00 it emulates the way BIOS
; loads the boot record into the memory.

; the output of this format is identical to format created from
; bin template, both templates create plain binary files.
; .boot output type is outdated because of its redundancy.
; you can write .bin file to boot record of a real
; floppy drive using writebin.asm (it's required to assemble this file first)

; note: you may not use dos interrupts for boot record code,
;       unless you manually replace them in interrupt vector table.
;       (refer to global memory map in the documentation)
                                          
                                          
#fasm#         ; use flat assembler syntax

#make_boot#

org 0x7c00

mov ax, 3h
int 10h

mov bx, 0
mov ah, 0x0e

mov si, string
call .print_string

mov si, capo
call .print_string

mov si, string2
call .print_string

.loop_input:
	call .input_char
	call .print_char
	cmp al, 13
	je .end
	jmp .loop_input

jmp .end

.end:
	jmp $

.print_string:
    mov bx, 0 
.print_string_loop:
    mov al, [si+bx]
    cmp al, 0
    je .return
    mov ah, 0x0e  
    int 10h 
    inc bx
    jmp .print_string_loop

.return:
    ret

.print_char:
	mov ah, 0x0e
	int 10h
	jmp .return

.input_char:
	mov ah, 0
	int 16h
	jmp .return

string db "Inserire password: ",0
string2 db "Inserire seconda password: ",0
capo db 10,13,0
