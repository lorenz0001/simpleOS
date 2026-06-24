#ifndef IDT_H
#define IDT_H

// Struttura dei registri che rispecchia il layout dello stack preparato dall'assembly (pushad/popad)
typedef struct registers {
    unsigned int ds;                                      // Selettore del segmento dati (DS)
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  // Spinti dall'istruzione assembly pushad
    unsigned int int_no, err_code;                        // Numero di interrupt e codice di errore (CPU/Stub)
    unsigned int eip, cs, eflags, useresp, ss;            // Spinti automaticamente dal processore al momento dell'interrupt
} registers_t;

// Struttura di una singola entry (Gate) della IDT (Interrupt Descriptor Table)
struct idt_entry_struct {
    unsigned short low_offset;   // I 16 bit inferiori dell'indirizzo a cui saltare
    unsigned short sel;          // Selettore del segmento di codice nella GDT (es. 0x08)
    unsigned char always0;       // Campo riservato che deve sempre essere impostato a 0
    unsigned char flags;         // Flag che definiscono il tipo di gate e il livello di privilegio (DPL)
    unsigned short high_offset;  // I 16 bit superiori dell'indirizzo a cui saltare
} __attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

// Struttura del registro IDTR che punta alla tabella IDT per l'istruzione lidt
struct idt_ptr_struct {
    unsigned short limit;        // Dimensione totale in byte della tabella IDT - 1
    unsigned int base;           // Indirizzo lineare/fisico di partenza della tabella IDT
} __attribute__((packed));

typedef struct idt_ptr_struct idt_ptr_t;

// Inizializza la IDT configurando le eccezioni della CPU e i vettori del PIC
void idt_init(void);

// Configura un singolo gate della IDT associando un vettore al relativo stub assembly
void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags);

// Dichiarazione degli handler C invocati dagli stub di basso livello in Assembly
// I parametri sono passati per VALORE: l'assembly prepara la struct sullo stack
// e il compilatore C la legge direttamente come parametro.
void isr_handler(registers_t regs);
void irq_handler(registers_t regs);

#endif
