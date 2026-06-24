#include "ports.h"
#include "screen.h"
#include "keyboard.h"
#include "string.h"

// Reboot CPU via keyboard controller CPU reset line
void reboot() {
    print_string("Resetting CPU via 8042 PS/2 controller...\n");
    for (volatile int i = 0; i < 0x2000000; i++); // Short busy-wait delay
    port_byte_out(0x64, 0xFE);                   // Pulse CPU reset line
}

// Parse and run shell commands in C
void process_command(const char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        print_string("Available Protected Mode commands:\n");
        print_string("  help   - Show this bare-metal command list\n");
        print_string("  info   - Display raw processor state information\n");
        print_string("  clear  - Clear screen (direct VGA memory write)\n");
        print_string("  exit   - Reboot back to 16-bit Bootloader Mode\n");
        print_string("  reboot - Pulse reset line to reboot computer\n");
    } else if (strcmp(cmd, "info") == 0) {
        print_string("System State Information (32-bit PMODE C-Kernel):\n");
        print_string("  CPU Mode: 32-bit Protected Mode (compiled in C)\n");
        print_string("  Video RAM: 0xB8000 (direct VGA Text Buffer)\n");
        print_string("  Keyboard driver: direct port I/O 0x60/0x64 (polling)\n");
        print_string("  Interrupts: disabled (CLI)\n");
        print_string("  Code Segment Selector: 0x08\n");
        print_string("  Data Segment Selector: 0x10\n");
    } else if (strcmp(cmd, "clear") == 0) {
        clear_screen();
    } else if (strcmp(cmd, "reboot") == 0 || strcmp(cmd, "exit") == 0) {
        reboot();
    } else {
        print_string("Error: Command not found in C PMODE. Type 'help'.\n");
    }
}

// C Entry Point
void kernel_main() {
    clear_screen();
    print_string("=========================================\n");
    print_string("   Antigravity Kernel Space (32-bit C)   \n");
    print_string("   Direct Hardware Access - BIOS Disabled\n");
    print_string("=========================================\n");
    print_string("Type 'help' for bare-metal C commands.\n\n");
    
    char input_buffer[64];
    while (1) {
        print_string("kernel# ");
        read_line(input_buffer);
        if (input_buffer[0] != '\0') {
            process_command(input_buffer);
        }
    }
}
