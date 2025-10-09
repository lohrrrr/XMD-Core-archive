#include "vga.h"
#include "../kernel.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_ADDRESS 0xB8000
#define DEFAULT_COLOR 0x0F

static int cursor = 0;

static void disable_hw_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void VGA_Init() { cursor = 0; disable_hw_cursor(); }

void VGA_Clear() {
    VGA_ClearColor(DEFAULT_COLOR);
}

void VGA_ClearColor(unsigned char color) {
    volatile unsigned char* video = (volatile unsigned char*)VGA_ADDRESS;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video[i * 2] = ' ';
        video[i * 2 + 1] = color;
    }
    cursor = 0;
}

void VGA_NewLine() {
    int row = cursor / VGA_WIDTH;
    int new_row = row + 1;
    if (new_row >= VGA_HEIGHT) new_row = VGA_HEIGHT - 1;
    cursor = new_row * VGA_WIDTH;
}

void VGA_PrintColor(const char* str, unsigned char color) {
    volatile unsigned char* video = (volatile unsigned char*)VGA_ADDRESS;
    while (*str) {
        if (*str == '\n') { 
            VGA_NewLine(); 
            str++; 
            continue; 
        }
        video[cursor * 2] = *str;
        video[cursor * 2 + 1] = color;
        cursor++;
        if (cursor >= VGA_WIDTH * VGA_HEIGHT) cursor = 0;
        str++;
    }
}

void VGA_Print(const char* str) {
    VGA_PrintColor(str, DEFAULT_COLOR);
}
