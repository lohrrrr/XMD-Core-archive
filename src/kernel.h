#ifndef KERNEL_H
#define KERNEL_H

#include <stdbool.h>

void kmain(void);
void kprint(const char* str);
void kclear();
int strcmp_own_hell(const char* s1, const char* s2);
void outb(unsigned short port, unsigned char val);

#endif
