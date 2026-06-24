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

// Mapping of keys when Shift is pressed
static const char shift_map[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

// Mapping of keys when AltGr is pressed
static const char altgr_map[128] = {
    [0x03] = '@',  // AltGr + 2
    [0x04] = '#',  // AltGr + 3
    [0x08] = '{',  // AltGr + 7
    [0x09] = '[',  // AltGr + 8
    [0x0A] = ']',  // AltGr + 9
    [0x0B] = '}',  // AltGr + 0
    [0x10] = '@',  // AltGr + q
    [0x1A] = '[',  // AltGr + [ (è in IT)
    [0x1B] = ']',  // AltGr + ] (+ in IT)
    [0x27] = '@',  // AltGr + ; (ò in IT)
    [0x28] = '#'   // AltGr + ' (à in IT)
};

// Mapping of keys when both Shift and AltGr are pressed
static const char altgr_shift_map[128] = {
    [0x1A] = '{',  // AltGr + Shift + [ (è in IT)
    [0x1B] = '}'   // AltGr + Shift + ] (+ in IT)
};

// State variables for key modifiers
static int lshift_pressed = 0;
static int rshift_pressed = 0;
static int shift_pressed = 0;
static int altgr_pressed = 0;
static int extended = 0;

// Map hardware key press scancode to ASCII character
char scancode_to_ascii(unsigned char code) {
    // 1. Handle extended key prefix
    if (code == 0xE0) {
        extended = 1;
        return 0;
    }
    
    // 2. Track modifier state on press/release
    int is_release = (code & 0x80) != 0;
    unsigned char clean_code = code & 0x7F;
    
    if (extended) {
        extended = 0; // Clear extended flag immediately for subsequent bytes
        
        // Right Alt (AltGr) has scancode 0x38 in extended state
        if (clean_code == 0x38) {
            altgr_pressed = !is_release;
        }
        return 0;
    }
    
    // Non-extended modifier keys
    if (clean_code == 0x2A) { // Left Shift
        lshift_pressed = !is_release;
        shift_pressed = lshift_pressed || rshift_pressed;
        return 0;
    }
    if (clean_code == 0x36) { // Right Shift
        rshift_pressed = !is_release;
        shift_pressed = lshift_pressed || rshift_pressed;
        return 0;
    }
    
    // Ignore all other key release events
    if (is_release) {
        return 0;
    }
    
    // 3. Resolve the mapped key based on modifier states
    if (altgr_pressed && shift_pressed) {
        char c = altgr_shift_map[clean_code];
        if (c != 0) return c;
    }
    
    if (altgr_pressed) {
        char c = altgr_map[clean_code];
        if (c != 0) return c;
    }
    
    if (shift_pressed) {
        if (clean_code < 128) {
            return shift_map[clean_code];
        }
    } else {
        if (clean_code < 128) {
            return scancode_map[clean_code];
        }
    }
    
    return 0;
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
