#ifndef VIDEO_H
#define VIDEO_H

#include <stdbool.h>

struct EFI_BOOT_SERVICES;
extern struct EFI_BOOT_SERVICES* BS;

bool is_running_on_uefi(void);
void Video_Init(void);
void vprint(const char* str);
void vprint_color(const char* str, unsigned char color);
void vclear(void);
void vclear_color(unsigned char color);

#endif
