#include "idt.h"
#include "ports.h"
#include "screen.h"

// Funzione esterna definita in kernel.c per stampare valori esadecimali
extern void print_hex(unsigned int val);

// Definisce l'array della IDT (Interrupt Descriptor Table) con 256 gate (entrate)
idt_entry_t idt[256];
// Definisce la struttura del registro IDTR caricata dalla CPU tramite lidt
idt_ptr_t idt_reg;

// Contatore globale dei tick di sistema incrementato dall'interrupt del timer (PIT IRQ0)
unsigned int system_ticks = 0;

// Callback esterna definita nel driver della tastiera (keyboard.h) per gestire i tasti premuti
extern void keyboard_handler_callback(void);

// Dichiarazione degli stub assembly per le eccezioni della CPU (0 - 31)
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

// Dichiarazione degli stub assembly per le richieste di interrupt hardware (32 - 47)
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

// Introduce un breve ritardo di I/O scrivendo su una porta non utilizzata (0x80)
// Questo è fondamentale su architetture x86 per dare il tempo ai controller PIC lenti
// di elaborare i comandi di inizializzazione ed evitare che vadano persi.
static void io_wait(void) {
    port_byte_out(0x80, 0);
}

// Rimappa i vettori del controller PIC (8259 Programmable Interrupt Controller)
// In modalità reale, il PIC associa IRQ 0-7 ai vettori interrupt 8-15.
// Tuttavia in modalità protetta a 32 bit, i vettori 8-15 sono riservati alle eccezioni
// interne della CPU (es. Double Fault). Per evitare collisioni, rimappiamo il PIC master
// per iniziare dal vettore 32 (0x20) e il PIC slave dal vettore 40 (0x28).
static void remap_pic(void) {
    // Invia l'Initialization Command Word 1 (ICW1) a entrambi i PIC per iniziare la sequenza
    port_byte_out(0x20, 0x11); // Invia comando 0x11 (inizializzazione) al PIC Master
    io_wait();
    port_byte_out(0xA0, 0x11); // Invia comando 0x11 al PIC Slave
    io_wait();
    
    // Invia l'ICW2 per impostare gli offset dei vettori interrupt
    port_byte_out(0x21, 0x20); // Master: sposta IRQ 0-7 ai vettori 32-39 (0x20-0x27)
    io_wait();
    port_byte_out(0xA1, 0x28); // Slave: sposta IRQ 8-15 ai vettori 40-47 (0x28-0x2F)
    io_wait();
    
    // Invia l'ICW3 per configurare la connessione a cascata tra i due PIC
    port_byte_out(0x21, 0x04); // Indica al Master che il Slave è connesso sulla linea IRQ2 (0100b = 0x04)
    io_wait();
    port_byte_out(0xA1, 0x02); // Indica al Slave la sua identità di cascata (0x02)
    io_wait();
    
    // Invia l'ICW4 per abilitare la modalità 8086
    port_byte_out(0x21, 0x01); // Imposta la modalità x86 a 8086 per il Master
    io_wait();
    port_byte_out(0xA1, 0x01); // Imposta la modalità x86 a 8086 per il Slave
    io_wait();
    
    // Configura le maschere di interrupt (OCW1) per abilitare solo le periferiche desiderate
    // 0xFD (11111101b) sul Master disabilita il Timer (IRQ0, bit 0 = 1) ma tiene abilitata la Tastiera (IRQ1, bit 1 = 0)
    // Questo serve a diagnosticare ed evitare crash immediati causati dal timer tick prima che il sistema sia pronto.
    port_byte_out(0x21, 0xFD);
    io_wait();
    // 0xFF (11111111b) disabilita tutti gli interrupt gestiti dal PIC Slave
    port_byte_out(0xA1, 0xFF);
    io_wait();
}

// Configura un singolo gate della IDT associando un vettore all'indirizzo dello stub assembly
void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags) {
    idt[num].low_offset = base & 0xFFFF;             // I 16 bit inferiori dell'indirizzo dello stub
    idt[num].sel = sel;                             // Selettore del segmento di codice (0x08)
    idt[num].always0 = 0;                            // Campo fisso a 0
    idt[num].flags = flags;                          // Flag di tipo e livello di privilegio del gate
    idt[num].high_offset = (base >> 16) & 0xFFFF;    // I 16 bit superiori dell'indirizzo dello stub
}

// Messaggi descrittivi associati alle prime 32 eccezioni della CPU (0 - 31)
static const char* exception_messages[] = {
    "Division By Zero (Errore Divisione per Zero / Overflow)",
    "Debug (Debug Exception)",
    "Non Maskable Interrupt (Interrupt Non Mascherabile)",
    "Breakpoint (Punto di Interruzione)",
    "Into Detected Overflow (Rilevato Overflow)",
    "Out of Bounds (Indice Fuori dai Limiti)",
    "Invalid Opcode (Istruzione non Valida)",
    "No Coprocessor (Coprocessore Non Disponibile)",
    "Double Fault (Doppio Guasto)",
    "Coprocessor Segment Overrun (Superamento Segmento Coprocessore)",
    "Bad TSS (TSS non Valido)",
    "Segment Not Present (Segmento non Presente)",
    "Stack Fault (Errore del Segmento di Stack)",
    "General Protection Fault (Errore di Protezione Generale - GPF)",
    "Page Fault (Errore di Pagina)",
    "Unknown Interrupt (Interrupt Sconosciuto)",
    "Coprocessor Fault (Errore Coprocessore)",
    "Alignment Check (Errore di Allineamento)",
    "Machine Check (Errore Hardware)",
    "SIMD Floating-Point (Errore Floating-Point SIMD)",
    "Virtualization (Errore di Virtualizzazione)",
    "Control Protection (Protezione di Controllo)",
    "Reserved (Riservato)",
    "Reserved (Riservato)",
    "Reserved (Riservato)",
    "Reserved (Riservato)",
    "Reserved (Riservato)",
    "Reserved (Riservato)",
    "Reserved (Riservato)",
    "Reserved (Riservato)",
    "Security Exception (Eccezione di Sicurezza)",
    "Reserved (Riservato)"
};

// Handler C delle eccezioni della CPU (ISR). Viene chiamato quando avviene un errore grave
// per stampare un messaggio diagnostico di panico (Kernel Panic) e arrestare il sistema.
void isr_handler(registers_t regs) {
    print_string("\n*** KERNEL PANIC: CPU EXCEPTION ");
    
    // Converte il numero dell'eccezione (regs.int_no) in stringa per poterlo stampare
    char num_str[16];
    int num = regs.int_no;
    int idx = 0;
    if (num == 0) {
        num_str[idx++] = '0';
    } else {
        while (num > 0) {
            num_str[idx++] = (num % 10) + '0';
            num /= 10;
        }
    }
    num_str[idx] = '\0';
    
    // Inverte la stringa dei numeri convertiti
    for (int i = 0; i < idx / 2; i++) {
        char tmp = num_str[i];
        num_str[i] = num_str[idx - 1 - i];
        num_str[idx - 1 - i] = tmp;
    }
    
    print_string(num_str);
    print_string(" (");
    if (regs.int_no < 32) {
        print_string(exception_messages[regs.int_no]);
    } else {
        print_string("Unknown");
    }
    print_string(") ***\n");
    
    // Stampa informazioni di debug fondamentali per individuare l'istruzione responsabile
    print_string("  EIP: 0x");
    print_hex(regs.eip);
    print_string("  CS: 0x");
    print_hex(regs.cs);
    print_string("  EFLAGS: 0x");
    print_hex(regs.eflags);
    print_string("\n  ERR_CODE: 0x");
    print_hex(regs.err_code);
    print_string("  ESP: 0x");
    print_hex(regs.esp);
    print_string("\nSystem halted. Please reboot the VM.\n");
    
    // Arresta per sempre il processore mettendolo in stato di risparmio energetico
    while (1) {
        __asm__ volatile("hlt");
    }
}

// Handler C per gli interrupt hardware (IRQ) generati da timer, tastiera, ecc.
void irq_handler(registers_t regs) {
    if (regs.int_no == 32) {
        // Incrementa i tick globali di sistema su IRQ0 (Timer)
        system_ticks++;
    } else if (regs.int_no == 33) {
        // Chiama la callback della tastiera su IRQ1 (Tastiera premuta)
        keyboard_handler_callback();
    }
    
    // Invia il segnale EOI (End of Interrupt) al controller PIC per indicare
    // che l'evento è stato gestito e consentire l'elaborazione di nuovi interrupt.
    if (regs.int_no >= 40) {
        // Se l'interrupt proviene dallo Slave PIC (IRQ 8-15), resettiamo lo Slave
        port_byte_out(0xA0, 0x20);
        io_wait();
    }
    // Resettiamo sempre il Master PIC
    port_byte_out(0x20, 0x20);
    io_wait();
}

// Funzione principale che inizializza la tabella dei descrittori di interrupt (IDT)
// impostando tutti i vettori e caricandola infine nella CPU.
void idt_init(void) {
    // Imposta i limiti e la base del registro IDTR
    idt_reg.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_reg.base = (unsigned int)&idt;
    
    // Resetta a zero tutte le 256 entrate per evitare handler non inizializzati
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    // Esegue la rimappatura del controller PIC
    remap_pic();
    
    // Registra i gate per le 32 eccezioni della CPU (0-31)
    // 0x08 è il selettore del segmento codice GDT.
    // 0x8E definisce un Gate di Interrupt a 32 bit presente a livello Ring 0.
    idt_set_gate(0, (unsigned int)isr0, 0x08, 0x8E);
    idt_set_gate(1, (unsigned int)isr1, 0x08, 0x8E);
    idt_set_gate(2, (unsigned int)isr2, 0x08, 0x8E);
    idt_set_gate(3, (unsigned int)isr3, 0x08, 0x8E);
    idt_set_gate(4, (unsigned int)isr4, 0x08, 0x8E);
    idt_set_gate(5, (unsigned int)isr5, 0x08, 0x8E);
    idt_set_gate(6, (unsigned int)isr6, 0x08, 0x8E);
    idt_set_gate(7, (unsigned int)isr7, 0x08, 0x8E);
    idt_set_gate(8, (unsigned int)isr8, 0x08, 0x8E);
    idt_set_gate(9, (unsigned int)isr9, 0x08, 0x8E);
    idt_set_gate(10, (unsigned int)isr10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned int)isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned int)isr12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned int)isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned int)isr14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned int)isr15, 0x08, 0x8E);
    idt_set_gate(16, (unsigned int)isr16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned int)isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned int)isr18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned int)isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned int)isr20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned int)isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned int)isr22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned int)isr23, 0x08, 0x8E);
    idt_set_gate(24, (unsigned int)isr24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned int)isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned int)isr26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned int)isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned int)isr28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned int)isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned int)isr30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned int)isr31, 0x08, 0x8E);
    
    // Registra i gate per gli interrupt hardware (IRQ 0-15 mapped in vectors 32-47)
    idt_set_gate(32, (unsigned int)irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned int)irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned int)irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned int)irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned int)irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned int)irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned int)irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned int)irq7, 0x08, 0x8E);
    idt_set_gate(40, (unsigned int)irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned int)irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned int)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned int)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned int)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned int)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned int)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned int)irq15, 0x08, 0x8E);
    
    // Carica la IDT modificando il registro interno della CPU tramite inline assembly (lidt)
    __asm__ volatile("lidt %0" : : "m"(idt_reg));
}
