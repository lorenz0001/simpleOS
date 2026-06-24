#include "screen.h"
#include "ports.h"

volatile char* video_memory = (volatile char*)0xB8000;
int cursor_offset = 0;

void update_cursor() {
    int position = cursor_offset / 2; // Each cell is 2 bytes (character + attribute)
    
    // Command 0x0F: Low byte of cursor offset
    port_byte_out(0x3D4, 0x0F);
    port_byte_out(0x3D5, (unsigned char)(position & 0xFF));
    
    // Command 0x0E: High byte of cursor offset
    port_byte_out(0x3D4, 0x0E);
    port_byte_out(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

void clear_screen() {
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0x0A; // Light green text on black background
    }
    cursor_offset = 0;
    update_cursor();
}

void scroll_screen() {
    // Move lines 1-24 up to lines 0-23
    for (int i = 0; i < 24 * 80 * 2; i++) {
        video_memory[i] = video_memory[i + 160];
    }
    // Clear the last line (line 24)
    for (int i = 24 * 80 * 2; i < 25 * 80 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0x0A;
    }
    cursor_offset = 24 * 160; // Cursor set to start of line 24
    update_cursor();
}

void print_string(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            int row = cursor_offset / 160;
            cursor_offset = (row + 1) * 160;
        } else if (str[i] == '\r') {
            int row = cursor_offset / 160;
            cursor_offset = row * 160;
        } else {
            video_memory[cursor_offset] = str[i];
            video_memory[cursor_offset + 1] = 0x0A;
            cursor_offset += 2;
        }
        
        // Scroll screen if we run out of space
        if (cursor_offset >= 4000) {
            scroll_screen();
        }
        i++;
    }
    update_cursor();
}
