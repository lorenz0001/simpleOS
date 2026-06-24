# Antigravity Terminal OS - Registro dei Progressi

Questo file tiene traccia dei progressi nello sviluppo del nostro sistema operativo personalizzato a 16 e 32 bit, progettato per essere eseguito con QEMU su WSL.

---

## 🚀 Stato Attuale del Progetto
Il sistema operativo è stato strutturato con due ambienti principali: la modalità di avvio (**Bootloader Space**) e il kernel a basso livello (**Kernel Space**).

### Funzionalità Implementate:
1. **Bootloader (16-bit)**: Carica il kernel in memoria a `0x1000` ed esegue il salto.
2. **Bootloader Space (16-bit Real Mode)**:
   - Funziona sulla base di interrupt del BIOS.
   - Offre comandi standard: `help`, `hello`, `kernel`, `clear` e `reboot`.
   - Prompt aggiornato: `bootloader> `.
3. **Transizione a 32-bit Protected Mode (PMODE)**:
   - All'avvio del comando `kernel`, disabilita gli interrupt (`cli`).
   - Carica la **Global Descriptor Table (GDT)** con i segmenti di codice (0x08) e dati (0x10) flat.
   - Abilita il flag PE (Protection Enable) in `CR0`.
   - Esegue un far jump per aggiornare il registro `CS` ed entrare nella modalità protetta a 32 bit.
4. **Bare-metal Kernel Space (32-bit PMODE)**:
   - **Nessuna dipendenza dal BIOS**: gli interrupt del BIOS sono completamente disabilitati.
   - **Driver Video VGA Diretto**: Scrittura di caratteri e attributi di colore direttamente nella memoria video all'indirizzo fisico `0xB8000`.
   - **Driver Cursore VGA Diretto**: Controllo della posizione del cursore lampeggiante tramite la scrittura sulle porte I/O `0x3D4` e `0x3D5`.
   - **Driver Tastiera I/O Diretto**: Lettura dei tasti premuti interrogando lo stato della porta `0x64` e prelevando i codici scancode dalla porta `0x60`.
   - **Risolto bug di blocco (PMODE Stack Alignment & Scancodes)**:
     - Risolto il disallineamento dello stack in PMODE: nella routine 32-bit `strcmp_pm` i registri venivano inseriti a 32-bit (`push eax`) ma rimossi a 16-bit (`pop ax`), disallineando lo stack pointer (`esp`) di 2 byte e corrompendo l'indirizzo di ritorno dell'istruzione `ret` (causa del blocco del comando `help` e di qualsiasi comando digitato). Sostituito con `pop eax`.
     - Sistemato il parsing degli array di scancode per evitare interpretazioni errate del backslash `'\'` (ora sostituito con l'ASCII `92`), evitando direttive errate di continuazione di riga in NASM.
   - **Mappa Scancode standard US**: Converte i codici tasto hardware in caratteri ASCII (gestendo backspace, spazio ed enter).
   - **Driver di Riavvio Hardware**: Invia il segnale di reset della CPU scrivendo `0xFE` sulla porta `0x64` del controller 8042 (PS/2), per riavviare la macchina (comando `reboot` o `exit`).
   - Prompt: `kernel# `.

---

## 📂 Struttura del Progetto

Il progetto è organizzato nel seguente modo:

* **[Makefile](file:///c:/Users/Lorenzo/Desktop/os/Makefile)**: Automatizza la compilazione e l'avvio in QEMU.
* **`src/`**: File sorgenti assembly
  * **[bootloader.asm](file:///c:/Users/Lorenzo/Desktop/os/src/bootloader.asm)**: Il settore di avvio (settore 1 del floppy).
  * **[kernel.asm](file:///c:/Users/Lorenzo/Desktop/os/src/kernel.asm)**: Kernel diviso in due parti: 16-bit (avvio e Bootloader shell) e 32-bit (Protected Mode e Kernel shell).
  * **[print.asm](file:///c:/Users/Lorenzo/Desktop/os/src/print.asm)**: Stampa di testo a 16-bit via BIOS.
  * **[disk.asm](file:///c:/Users/Lorenzo/Desktop/os/src/disk.asm)**: Caricatore del disco a 16-bit via BIOS.
  * **[keyboard.asm](file:///c:/Users/Lorenzo/Desktop/os/src/keyboard.asm)**: Lettura tastiera a 16-bit via BIOS.
  * **[string.asm](file:///c:/Users/Lorenzo/Desktop/os/src/string.asm)**: Confronto stringhe a 16-bit.

---

## 🛠️ Come Compilare ed Eseguire

### 1. Installazione dei Prerequisiti (su WSL / Ubuntu)
```bash
sudo apt update
sudo apt install -y nasm qemu-system-x86
```

### 2. Compilazione
```bash
make
```

### 3. Esecuzione
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
- [ ] Implementare una GDT con livelli di privilegio separati.
- [ ] Aggiungere il supporto per gli interrupt a 32 bit tramite la IDT.
- [ ] Scrivere un linker script per poter compilare driver complessi in linguaggio C.
