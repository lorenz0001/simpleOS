#include "mem.h"

// Definizione dell'indirizzo fisico di partenza dell'heap (1MB in RAM)
// Evitiamo il primo MB di memoria fisica poiché contiene l'area IVT, il codice BIOS,
// il bootloader, lo stack e il codice del nostro stesso kernel caricato a 0x1000.
#define HEAP_START 0x100000 // Inizio a 1MB
// Dimensione dell'heap impostata a 1MB (copre da 1MB a 2MB fisici)
#define HEAP_SIZE  0x100000 

// Struttura dell'intestazione (Header) di ogni blocco di memoria dell'heap.
// Ogni blocco allocato o libero è preceduto da questa intestazione che descrive lo stato del blocco.
// Viene usata la direttiva packed per forzare il compilatore a non inserire padding interno.
typedef struct block_header {
    unsigned int size;            // Dimensione del blocco di dati effettivo in byte (escluso l'header)
    int is_free;                  // Flag di stato: 1 se il blocco è libero e riutilizzabile, 0 se allocato
    struct block_header* next;    // Puntatore alla prossima intestazione di blocco in memoria
} __attribute__((packed)) block_header_t;

// Puntatore fisso che fa riferimento al primo blocco di memoria all'inizio dell'heap.
static block_header_t* first_block = (block_header_t*)HEAP_START;

// Inizializza l'heap configurando il primo blocco gigante che rappresenta l'intera memoria libera.
void mem_init(void) {
    // La dimensione disponibile per i dati è la dimensione totale dell'heap meno la dimensione dell'header iniziale
    first_block->size = HEAP_SIZE - sizeof(block_header_t);
    first_block->is_free = 1; // Inizialmente tutto l'heap è marcato come libero
    first_block->next = 0;    // Non ci sono altri blocchi successivi per ora
}

// Alloca una porzione di memoria heap di dimensione 'size' byte utilizzando l'algoritmo First-Fit.
void* malloc(unsigned int size) {
    if (size == 0) return 0;
    
    // Allinea la dimensione a 4 byte per ottimizzare l'accesso in memoria della CPU a 32 bit.
    // L'allineamento a 32 bit (4 byte) evita letture/scritture non allineate che degradano le prestazioni.
    size = (size + 3) & ~3;
    
    block_header_t* curr = first_block;
    
    // Scorre la lista concatenata dei blocchi (First-Fit)
    while (curr != 0) {
        // Cerca il primo blocco che sia libero e abbastanza grande per ospitare la richiesta
        if (curr->is_free && curr->size >= size) {
            
            // Soglia minima di spazio necessaria per eseguire lo splitting (divisione del blocco).
            // Dobbiamo avere spazio sufficiente per inserire un nuovo header per la parte rimanente
            // e un payload dati minimo di almeno 4 byte.
            unsigned int threshold = sizeof(block_header_t) + 4;
            
            if (curr->size >= size + threshold) {
                // Esegue lo splitting: divide il blocco corrente in due parti.
                // 1. Calcola l'indirizzo di partenza del nuovo blocco rimanente (subito dopo i dati di quello corrente)
                block_header_t* new_block = (block_header_t*)((char*)curr + sizeof(block_header_t) + size);
                
                // 2. Imposta la dimensione del nuovo blocco (dimensione precedente meno i dati allocati e il nuovo header)
                new_block->size = curr->size - size - sizeof(block_header_t);
                new_block->is_free = 1; // La parte rimanente rimane libera
                new_block->next = curr->next; // Collega il nuovo blocco alla lista successiva
                
                // 3. Ridimensiona il blocco corrente alla dimensione esatta richiesta
                curr->size = size;
                curr->next = new_block; // Collega il blocco corrente al nuovo blocco appena creato
            }
            
            // Marca il blocco come allocato (non più libero)
            curr->is_free = 0; 
            
            // Ritorna il puntatore all'area dati effettiva del blocco (saltando l'header iniziale)
            return (void*)((char*)curr + sizeof(block_header_t));
        }
        curr = curr->next;
    }
    
    return 0; // Ritorna NULL se non c'è abbastanza memoria libera (Out of Memory)
}

// Rilascia un blocco di memoria precedentemente allocato e fonde i blocchi liberi adiacenti (Coalescing).
void free(void* ptr) {
    if (ptr == 0) return;
    
    // Risale all'intestazione del blocco sottraendo la dimensione dell'header dall'indirizzo del puntatore dati
    block_header_t* block = (block_header_t*)((char*)ptr - sizeof(block_header_t));
    block->is_free = 1; // Marca il blocco come libero
    
    // Algoritmo di Coalescenza (Coalescing): scorre l'intera lista dei blocchi
    // e unisce immediatamente qualsiasi coppia di blocchi liberi contigui per combattere la frammentazione.
    block_header_t* curr = first_block;
    while (curr != 0) {
        // Se il blocco corrente è libero, c'è un blocco successivo, ed anche il successivo è libero
        if (curr->is_free && curr->next != 0 && curr->next->is_free) {
            // Fonde il blocco corrente con il blocco successivo sommandone le dimensioni
            // ed incorporando lo spazio occupato dall'header del blocco rimosso.
            curr->size = curr->size + sizeof(block_header_t) + curr->next->size;
            curr->next = curr->next->next; // Salta il blocco successivo fuso
            
            // Continua il ciclo senza avanzare per verificare se anche il nuovo blocco successivo è libero
            continue;
        }
        curr = curr->next;
    }
}
