// Vga "Driver" API
// Version:0.7
// Author:Lohrrrr

#ifndef VGA_H
#define VGA_H

// Text mode functions
void VGA_Init();
void VGA_Clear();
void VGA_Print(const char* str);
void VGA_PrintColor(const char* str, unsigned char color);
void VGA_ClearColor(unsigned char color);

// Mode switching
void VGA_SetMode13h();  // 320x200 256colors
void VGA_SetMode12h();  // 640x480 16cololrd
void VGA_SetTextMode(); // 80x25 text

// 13h mode functions (320x200x256) (placeholder)
void VGA_PutPixel13h(int x, int y, unsigned char color);
void VGA_DrawLine13h(int x1, int y1, int x2, int y2, unsigned char color);
void VGA_DrawRect13h(int x, int y, int w, int h, unsigned char color);
void VGA_DrawText13h(int x, int y, const char* str, unsigned char color);

// 12h mode functions (640x480x16) (placeholder)
void VGA_PutPixel12h(int x, int y, unsigned char color);
void VGA_DrawLine12h(int x1, int y1, int x2, int y2, unsigned char color);
void VGA_DrawRect12h(int x, int y, int w, int h, unsigned char color);
void VGA_DrawText12h(int x, int y, const char* str, unsigned char color);

#endif
