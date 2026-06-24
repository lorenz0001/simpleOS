# Antigravity Terminal OS - Registro dei Progressi

Questo file tiene traccia dei progressi nello sviluppo del nostro sistema operativo personalizzato a 16 e 32 bit, progettato per essere eseguito con QEMU su WSL.

---

## 🚀 Stato Attuale del Progetto
Abbiamo riprogettato il kernel introducendo un'importante astrazione: il kernel a 32 bit in modalità protetta è stato migrato da codice Assembly puro a **linguaggio C** (`kernel.c`). Le funzionalità di basso livello (interazione con le porte hardware) sono fornite da una libreria assembly apposita (`ports.asm`).

### Funzionalità Implementate:
1. **Bootloader (16-bit)**: Carica il kernel all'indirizzo fisico `0x1000` e salta ad esso.
2. **Bootloader Space (16-bit Real Mode)**:
   - Basato su interrupt del BIOS.
   - Fornisce la shell interattiva `bootloader> ` con i comandi: `help`, `hello`, `kernel`, `clear`, `reboot`.
3. **Transizione a 32-bit Protected Mode (PMODE)**:
   - Configura la **GDT** (Global Descriptor Table).
   - Imposta il flag PE in `CR0`, disabilita gli interrupt (`cli`) ed effettua un far jump caricando il selettore di codice `0x08` e inizializzando lo stack pointer a `0x90000`.
   - Chiama l'entry point del kernel C: `kernel_main()`.
4. **C-Based Kernel Space (32-bit PMODE)**:
   - **Linguaggio C**: Il ciclo shell e la logica dei comandi sono scritti interamente in C.
   - **Libreria Assembly I/O (`ports.asm`)**: Espone le funzioni a 32-bit `port_byte_in` e `port_byte_out` secondo lo standard C (`cdecl`).
   - **Driver Video VGA in C**: Gestione diretta dell'area di memoria `0xB8000`. Scrittura di caratteri, attributi di colore, pulizia dello schermo (`clear_screen`), scorrimento del testo (`scroll_screen`) e aggiornamento del cursore lampeggiante (`update_cursor`).
   - **Driver Tastiera I/O in C**: Funzione di scansione della porta di stato `0x64` e lettura del codice hardware dalla porta `0x60`, convertito tramite tabella di traduzione US. Gestione del backspace e invio in C.
   - Prompt: `kernel# `.
   - Comandi supportati: `help`, `info`, `clear`, `exit` (reboot), `reboot`.

---

## 📂 Struttura del Progetto

Il progetto è organizzato nel seguente modo:

* **[Makefile](file:///c:/Users/Lorenzo/Desktop/os/Makefile)**: Automatizza la compilazione dei file assembly in ELF32, dei file C in ELF32 e il collegamento in un binario flat unico tramite `ld`.
* **`src/`**: File sorgenti
  * **[bootloader.asm](file:///c:/Users/Lorenzo/Desktop/os/src/boot/bootloader.asm)**: Il settore di avvio.
  * **[kernel.asm](file:///c:/Users/Lorenzo/Desktop/os/src/kernel/kernel.asm)**: Inizializza la shell a 16 bit, configura la GDT, esegue lo switch a PMODE e salta a `kernel_main`.
  * **[kernel.c](file:///c:/Users/Lorenzo/Desktop/os/src/kernel/kernel.c)**: Entry point del kernel a 32 bit e CLI shell loop.
  * **[ports.asm](file:///c:/Users/Lorenzo/Desktop/os/src/cpu/ports.asm)** / **[ports.h](file:///c:/Users/Lorenzo/Desktop/os/src/cpu/ports.h)**: Interfaccia assembly per letture/scritture I/O porte hardware.
  * **[screen.c](file:///c:/Users/Lorenzo/Desktop/os/src/drivers/screen.c)** / **[screen.h](file:///c:/Users/Lorenzo/Desktop/os/src/drivers/screen.h)**: Driver video VGA in C (memoria 0xB8000, cursore, scorrimento).
  * **[keyboard.c](file:///c:/Users/Lorenzo/Desktop/os/src/drivers/keyboard.c)** / **[keyboard.h](file:///c:/Users/Lorenzo/Desktop/os/src/drivers/keyboard.h)**: Driver tastiera in C (scansione porte e buffer).
  * **[string.c](file:///c:/Users/Lorenzo/Desktop/os/src/libc/string.c)** / **[string.h](file:///c:/Users/Lorenzo/Desktop/os/src/libc/string.h)**: Funzione C standard di comparazione stringhe (`strcmp`).
  * **[print.asm](file:///c:/Users/Lorenzo/Desktop/os/src/boot/print.asm)**, **[disk.asm](file:///c:/Users/Lorenzo/Desktop/os/src/boot/disk.asm)**, **[keyboard.asm](file:///c:/Users/Lorenzo/Desktop/os/src/boot/keyboard.asm)**, **[string.asm](file:///c:/Users/Lorenzo/Desktop/os/src/boot/string.asm)**: Librerie assembly per la modalità a 16 bit.

---

## 🛠️ Come Compilare ed Eseguire

### 1. Compilazione
Assicurati di essere in WSL ed esegui:
```bash
make clean
make
```

### 2. Esecuzione
Avvia il sistema in una finestra separata tramite WSLg:
```bash
make run
```
Oppure nel terminale stesso tramite curses:
```bash
make run-curses
```

---

## 📈 Prossimi Passi (Roadmap)
- [ ] Implementazione del supporto per gli interrupt a 32 bit tramite IDT (Interrupt Descriptor Table).
- [ ] Aggiunta di driver di memoria più complessi (malloc bare-metal).
