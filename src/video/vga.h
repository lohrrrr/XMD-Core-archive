#ifndef VGA_H
#define VGA_H

void VGA_Init();
void VGA_Clear();
void VGA_Print(const char* str);
void VGA_PrintColor(const char* str, unsigned char color);
void VGA_ClearColor(unsigned char color);

#endif
