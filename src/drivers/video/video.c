// Unique Video Protocol
// Version:0.4
// Author:Lohrrrr
// this file is probably will not get any updates in some time

#include "vga/vga.h"
#include "video.h"

int IS_UEFI = 0; // it will be used later, someday...

void Video_Init(void) {
    if (!IS_UEFI) {VGA_Init();} else {/*GOP_Init();*/}
}

void vprint_color(const char* str, unsigned char color) {
    if (!IS_UEFI) {VGA_PrintColor(str, color);} else {/*GOP_PrintColor(str, color);*/}
}

void vprint(const char* str) {
    if (!IS_UEFI) {VGA_Print(str);} else {/*GOP_Print(str);*/}
}

void vclear(void) {
    if (!IS_UEFI) {VGA_Clear();} else {/*GOP_Clear();*/}
}

void vclear_color(unsigned char color) {
    if (!IS_UEFI) {VGA_ClearColor(color);} else {/*GOP_ClearColor(color);*/}
}

