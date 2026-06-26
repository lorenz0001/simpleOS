#ifndef SCREEN_H
#define SCREEN_H

#include "system.h"

int cursor_row = 0;
int cursor_column = 0;

void update_cursor(int x, int y)
{
        uint16 pos = y * 80 + x;

        outb(0x3D4, 0x0F);
        outb(0x3D5, (uint8)(pos & 0xFF));
        outb(0x3D4, 0x0E);
        outb(0x3D5, (uint8)((pos >> 8) & 0xFF));
}

void disable_cursor()
{
        outb(0x3D4, 0x0A);
        outb(0x3D5, 0x20);
}

void enable_cursor(uint8 cursor_start, uint8 cursor_end)
{
        outb(0x3D4, 0x0A);
        outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

        outb(0x3D4, 0x0B);
        outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void scroll_screen()
{
        volatile char* vidmem = (volatile char*)0xb8000;
        for (int i = 0; i < 24 * 80 * 2; i++) {
                vidmem[i] = vidmem[i + 80 * 2];
        }
        for (int i = 24 * 80 * 2; i < 25 * 80 * 2; i += 2) {
                vidmem[i] = ' ';
                vidmem[i + 1] = 0x07;
        }
        cursor_row = 24;
}

void wipe_screen()
{
        volatile char* vidmem = (volatile char*)0xb8000;
        int total_chars = 80 * 25 * 2;
        for (int i = 0; i < total_chars; i += 2) {
                vidmem[i] = ' ';
                vidmem[i + 1] = 0x07;
        }
        cursor_row = 0;
        cursor_column = 0;
        update_cursor(0, 0);
}

void new_line()
{
        cursor_column = 0;
        cursor_row++;
        if (cursor_row >= 25) {
                scroll_screen();
        }
        update_cursor(cursor_column, cursor_row);
}

void print_char(char carattere)
{
        if (carattere == '\n') {
                new_line();
                return;
        }
        if (carattere == '\r') {
                cursor_column = 0;
                update_cursor(cursor_column, cursor_row);
                return;
        }

        volatile char* vidmem = (volatile char*)0xb8000;
        int offset = (cursor_row * 80 + cursor_column) * 2;
        vidmem[offset] = carattere;
        vidmem[offset + 1] = 0x07;

        cursor_column++;
        if (cursor_column >= 80) {
                new_line();
        } else {
                update_cursor(cursor_column, cursor_row);
        }
}

void print_string(string stringa)
{
        int i = 0;
        while (stringa[i] != '\0') {
                print_char(stringa[i]);
                i++;
        }
}

#endif
