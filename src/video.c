#include "video/vga.h"
#include "video/gop.h"
#include "video.h"

bool is_running_on_uefi(void) {
    //return BS != NULL;
    return 0;
}

void Video_Init(void) {
    if (is_running_on_uefi()) {
        GOP_Init();
    } else {
        VGA_Init();
    }
}

void vprint_color(const char* str, unsigned char color) {
    if (is_running_on_uefi()) { GOP_PrintColor(str, color); } else { VGA_PrintColor(str, color); }
}

void vprint(const char* str) {
    if (is_running_on_uefi()) { GOP_Print(str); } else { VGA_Print(str); }
}

void vclear(void) {
    if (is_running_on_uefi()) { GOP_Clear(); } else { VGA_Clear(); }
}

void vclear_color(unsigned char color) {
    if (is_running_on_uefi()) { GOP_ClearColor(color); } else { VGA_ClearColor(color); }
}

