#ifndef KEYBOARD_H
#define KEYBOARD_H

unsigned char read_key(void);
void flush_keyboard(void);
char scancode_to_ascii(unsigned char code);
void read_line(char* buffer);

#endif
