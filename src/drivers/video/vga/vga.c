// Vga "Driver"
// Version:0.8
// Author:Lohrrrr

#include "vga.h"
#include "../../../api.h"
#include "basic_font.h"

// TEXT-MODE + BASE SECTION

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_ADDRESS 0xB8000
#define DEFAULT_COLOR 0x0F

static int cursor = 0;

static void disable_hw_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void VGA_SetTextMode() {
    asm volatile (
        "mov $0x03, %%ax\n\t"
        "int $0x10"
        :
        :
        : "ax"
    );
}

void VGA_Init() { cursor = 0; disable_hw_cursor(); }

void VGA_Clear() { VGA_ClearColor(DEFAULT_COLOR); }

void VGA_ClearColor(unsigned char color) {
    volatile unsigned char* video = (volatile unsigned char*)VGA_ADDRESS;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video[i * 2] = ' ';
        video[i * 2 + 1] = color;
    }
    cursor = 0;
}

void VGA_ScrollUp() {
    volatile unsigned char* video = (volatile unsigned char*)VGA_ADDRESS;
    for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
        video[i * 2] = video[(i + VGA_WIDTH) * 2];
        video[i * 2 + 1] = video[(i + VGA_WIDTH) * 2 + 1];
    }
    for (int i = VGA_WIDTH * (VGA_HEIGHT - 1); i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video[i * 2] = ' ';
        video[i * 2 + 1] = DEFAULT_COLOR;
    }
    cursor = VGA_WIDTH * (VGA_HEIGHT - 1);
}

void VGA_NewLine() {
    int row = cursor / VGA_WIDTH;
    int new_row = row + 1;
    if (new_row >= VGA_HEIGHT) {
        VGA_ScrollUp();
    } else {
        cursor = new_row * VGA_WIDTH;
    }
}

void VGA_PrintColor(const char* str, unsigned char color) {
    volatile unsigned char* video = (volatile unsigned char*)VGA_ADDRESS;
    while (*str) {
        if (*str == '\n') {
            VGA_NewLine();
            str++;
            continue;
        }
        if (*str == '\b') {
            if (cursor > 0) {
                cursor--;
                video[cursor * 2] = ' ';
                video[cursor * 2 + 1] = color;
            }
            str++;
            continue;
        }
        video[cursor * 2] = *str;
        video[cursor * 2 + 1] = color;
        cursor++;
        if (cursor >= VGA_WIDTH * VGA_HEIGHT) {
            VGA_ScrollUp();
        }
        str++;
    }
}

void VGA_Print(const char* str) { VGA_PrintColor(str, DEFAULT_COLOR); }

// 12TH-MODE SECTION

// HONORABLE MENTION:
// I'm probably goin' to kill my Gt640 with this VGA mode and then ask my friend for their Gt240 to continiune testing on real hardware
// Parts of code that CAN kill hardware comented with "//@ 12th-can-cause-damage", use it only when u know what u doing

#define VGA_MEMORY ((volatile unsigned char*)0xA0000)

void VGA_SetMode12h() {
    asm volatile (
        "mov $0x12, %%ax\n\t"
        "int $0x10"
        :
        :
        : "ax"
    );
}


void VGA_PutPixel12h(int x, int y, unsigned char color) {
    if (x < 0 || x >= 640 || y < 0 || y >= 480) return;

    unsigned short offset = (y * 80) + (x / 8);
    unsigned char mask = 0x80 >> (x % 8);

    volatile unsigned char* vram = (unsigned char*)0xA0000;

    outb(0x3CE, 0x08);
    outb(0x3CF, mask);

    outb(0x3CE, 0x00);
    outb(0x3CF, color & 0x0F);

    vram[offset] = 0xFF;
}

void VGA_DrawLine12h(int x1, int y1, int x2, int y2, unsigned char color) {
    int dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int sx = (x1 < x2) ? 1 : -1;
    int dy = (y2 > y1) ? -(y2 - y1) : -(y1 - y2);
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx + dy;

    while (1) {
        VGA_PutPixel12h(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

void VGA_DrawRect12h(int x, int y, int w, int h, unsigned char color) {
    VGA_DrawLine12h(x, y, x + w - 1, y, color);
    VGA_DrawLine12h(x, y, x, y + h - 1, color);
    VGA_DrawLine12h(x + w - 1, y, x + w - 1, y + h - 1, color);
    VGA_DrawLine12h(x, y + h - 1, x + w - 1, y + h - 1, color);
}

//@ 12th-can-cause-damage
// Reason:This code were taken from OSDev forum, still untested

void VGA_DrawText12h(int x, int y, const char* str, unsigned char color) {
    while (*str) {
        unsigned char c = *str++;
        if (c >= 128) continue;

        unsigned char* glyph = font8x8_basic[c];
        for (int cy = 0; cy < 8; cy++) {
            unsigned char line = glyph[cy];
            for (int cx = 0; cx < 8; cx++) {
                if (line & (1 << (7 - cx))) {
                    VGA_PutPixel12h(x + cx, y + cy, color);
                }
            }
        }
        x += 8;
    }
}

//@ end

