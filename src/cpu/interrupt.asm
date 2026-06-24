[bits 32]

; Define external C handlers
extern isr_handler
extern irq_handler

; Exception stubs (0 - 31)
global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

; IRQ stubs (32 - 47)
global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

; Exception Stub Definitions
; 0: Divide By Zero Exception
isr0:
    push byte 0
    push byte 0
    jmp isr_common_stub

; 1: Debug Exception
isr1:
    push byte 0
    push byte 1
    jmp isr_common_stub

; 2: Non Maskable Interrupt Exception
isr2:
    push byte 0
    push byte 2
    jmp isr_common_stub

; 3: Int 3 Exception
isr3:
    push byte 0
    push byte 3
    jmp isr_common_stub

; 4: INTO Exception
isr4:
    push byte 0
    push byte 4
    jmp isr_common_stub

; 5: Out of Bounds Exception
isr5:
    push byte 0
    push byte 5
    jmp isr_common_stub

; 6: Invalid Opcode Exception
isr6:
    push byte 0
    push byte 6
    jmp isr_common_stub

; 7: Coprocessor Not Available Exception
isr7:
    push byte 0
    push byte 7
    jmp isr_common_stub

; 8: Double Fault Exception (pushes error code)
isr8:
    push byte 8
    jmp isr_common_stub

; 9: Coprocessor Segment Overrun Exception
isr9:
    push byte 0
    push byte 9
    jmp isr_common_stub

; 10: Bad TSS Exception (pushes error code)
isr10:
    push byte 10
    jmp isr_common_stub

; 11: Segment Not Present Exception (pushes error code)
isr11:
    push byte 11
    jmp isr_common_stub

; 12: Stack Fault Exception (pushes error code)
isr12:
    push byte 12
    jmp isr_common_stub

; 13: General Protection Fault Exception (pushes error code)
isr13:
    push byte 13
    jmp isr_common_stub

; 14: Page Fault Exception (pushes error code)
isr14:
    push byte 14
    jmp isr_common_stub

; 15: Unknown Interrupt Exception
isr15:
    push byte 0
    push byte 15
    jmp isr_common_stub

; 16: Coprocessor Fault Exception
isr16:
    push byte 0
    push byte 16
    jmp isr_common_stub

; 17: Alignment Check Exception (pushes error code)
isr17:
    push byte 17
    jmp isr_common_stub

; 18: Machine Check Exception
isr18:
    push byte 0
    push byte 18
    jmp isr_common_stub

; 19: SIMD Floating-Point Exception
isr19:
    push byte 0
    push byte 19
    jmp isr_common_stub

; 20: Virtualization Exception
isr20:
    push byte 0
    push byte 20
    jmp isr_common_stub

; 21: Control Protection Exception (pushes error code)
isr21:
    push byte 21
    jmp isr_common_stub

; 22 to 31: Reserved Exceptions
isr22: push byte 0; push byte 22; jmp isr_common_stub
isr23: push byte 0; push byte 23; jmp isr_common_stub
isr24: push byte 0; push byte 24; jmp isr_common_stub
isr25: push byte 0; push byte 25; jmp isr_common_stub
isr26: push byte 0; push byte 26; jmp isr_common_stub
isr27: push byte 0; push byte 27; jmp isr_common_stub
isr28: push byte 0; push byte 28; jmp isr_common_stub
isr29: push byte 0; push byte 29; jmp isr_common_stub
isr30: push byte 30; jmp isr_common_stub ; Security exception (pushes error code)
isr31: push byte 0; push byte 31; jmp isr_common_stub

; Hardware IRQ Stub Definitions
irq0:  push byte 0; push byte 32; jmp irq_common_stub
irq1:  push byte 0; push byte 33; jmp irq_common_stub
irq2:  push byte 0; push byte 34; jmp irq_common_stub
irq3:  push byte 0; push byte 35; jmp irq_common_stub
irq4:  push byte 0; push byte 36; jmp irq_common_stub
irq5:  push byte 0; push byte 37; jmp irq_common_stub
irq6:  push byte 0; push byte 38; jmp irq_common_stub
irq7:  push byte 0; push byte 39; jmp irq_common_stub
irq8:  push byte 0; push byte 40; jmp irq_common_stub
irq9:  push byte 0; push byte 41; jmp irq_common_stub
irq10: push byte 0; push byte 42; jmp irq_common_stub
irq11: push byte 0; push byte 43; jmp irq_common_stub
irq12: push byte 0; push byte 44; jmp irq_common_stub
irq13: push byte 0; push byte 45; jmp irq_common_stub
irq14: push byte 0; push byte 46; jmp irq_common_stub
irq15: push byte 0; push byte 47; jmp irq_common_stub

; =====================================================================
; Stub Comune per le Eccezioni (ISR)
; Dopo pushad e push ds, lo stack contiene esattamente la struttura registers_t.
; La funzione C isr_handler(registers_t regs) legge i parametri direttamente dallo stack.
; =====================================================================
isr_common_stub:
    pushad              ; Salva EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX (32 byte)
    
    mov ax, ds          ; Salva il segmento dati corrente
    push eax            ; Spinge DS sullo stack (4 byte)
    
    mov ax, 0x10        ; Carica il selettore del segmento dati del kernel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Chiama l'handler C. La struttura registers_t è già sullo stack
    ; e viene letta direttamente dal compilatore come parametro passato per valore.
    call isr_handler
    
    pop eax             ; Ripristina il segmento dati originale
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popad               ; Ripristina i registri generali
    add esp, 8          ; Rimuove int_no e err_code dallo stack
    iret                ; Ritorna dall'interrupt

; =====================================================================
; Stub Comune per le Richieste di Interrupt Hardware (IRQ)
; =====================================================================
irq_common_stub:
    pushad              ; Salva i registri generali
    
    mov ax, ds          ; Salva DS
    push eax            
    
    mov ax, 0x10        ; Carica il segmento dati del kernel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Chiama l'handler C per gli interrupt hardware
    call irq_handler
    
    pop eax             ; Ripristina DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popad               ; Ripristina i registri generali
    add esp, 8          ; Rimuove err_code fittizio e numero IRQ
    iret
