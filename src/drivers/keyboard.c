#include "keyboard.h"
#include "ports.h"
#include "screen.h"

extern volatile char* video_memory;

unsigned char read_key() {
    while ((port_byte_in(0x64) & 1) == 0); // Loop until Output Buffer Full (OBF) bit is 1
    return port_byte_in(0x60);             // Read the key code from data register
}

void flush_keyboard() {
    while (port_byte_in(0x64) & 1) {
        port_byte_in(0x60);
    }
}

// Translation table for standard US Keyboard Layout
static const char scancode_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

// Map hardware key press scancode to ASCII character
char scancode_to_ascii(unsigned char code) {
    if (code >= 0x80) return 0; // Ignore break codes (key release events)
    return scancode_map[code];
}

// Read a full command line from keyboard
void read_line(char* buffer) {
    int len = 0;
    while (1) {
        unsigned char scancode = read_key();
        char ascii = scancode_to_ascii(scancode);
        if (ascii == 0) continue;
        
        // Enter key ends line
        if (ascii == '\n') {
            buffer[len] = '\0';
            print_string("\n");
            break;
        }
        
        // Handle backspace
        if (ascii == '\b') {
            if (len > 0) {
                len--;
                cursor_offset -= 2;
                video_memory[cursor_offset] = ' ';
                video_memory[cursor_offset + 1] = 0x0A;
                update_cursor();
            }
            continue;
        }
        
        // Append character if space is available
        if (len < 63) {
            buffer[len] = ascii;
            len++;
            char echo[2] = {ascii, '\0'};
            print_string(echo);
        }
    }
}
