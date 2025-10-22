// Unique Video Protocol
// Version:0.4
// Author:Lohrrrr

#include "vga/vga.h"
#include "video.h"

void Video_Init(void) {
    VGA_Init();
}

void vprint_color(const char* str, unsigned char color) {
    VGA_PrintColor(str, color);
}

void vprint(const char* str) {
    VGA_Print(str);
}

void vclear(void) {
    VGA_Clear();
}

void vclear_color(unsigned char color) {
    VGA_ClearColor(color);
}

