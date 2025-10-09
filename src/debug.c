#include "debug.h"
#include "video/vga.h"

int debug_prints_en = 1;
typedef unsigned char uint8_t;

static void PrintBorder(uint8_t color);
static void PrintCentered(const char* text, uint8_t color);

static void PrintBorder(uint8_t color) {
    VGA_PrintColor("========================================\n", color);
}

static void PrintCentered(const char* text, uint8_t color) {
    int len = 0;
    while (text[len]) len++;
    int padding = (40 - len) / 2;
    for (int i = 0; i < padding; i++) VGA_PrintColor(" ", color);
    VGA_PrintColor(text, color);
    VGA_PrintColor("\n", color);
}

void Debug_RSOF(const char* reason) {
    VGA_Clear();

    PrintBorder(0x2);
    PrintCentered("!!! DEBUG HALT !!!", 0x2);
    PrintBorder(0x2);

    VGA_PrintColor("Reason: ", 0x2);
    VGA_PrintColor(reason, 0x2);
    VGA_PrintColor("\n\nThis halt was triggered from debug function.\n", 0x2);

    PrintBorder(0x2);

    for (;;) __asm__ __volatile__("hlt");
}

void RSOF(const char* reason) {
    VGA_ClearColor(0x4F);

    PrintBorder(0x4F);
    PrintCentered("*** FATAL SYSTEM ERROR ***", 0x4F);
    PrintBorder(0x4F);

    VGA_PrintColor("Fatal error: ", 0x4F);
    VGA_PrintColor(reason, 0x4F);
    VGA_PrintColor("\n\nSystem halted. DO PANIC.\n", 0x4F);

    PrintBorder(0x4F);

    for (;;) __asm__ __volatile__("hlt");
}

void DPRINT(const char* text) {
    if (debug_prints_en) {
        VGA_PrintColor("[DEBUG]: ", 0x2);
        VGA_Print(text);
        VGA_Print("\n");
    }
}
