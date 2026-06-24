#ifndef SCREEN_H
#define SCREEN_H

extern int cursor_offset;

void update_cursor(void);
void clear_screen(void);
void scroll_screen(void);
void print_string(const char* str);

#endif
