// Main Kernel
// Version:0.6 (full rewrite)
// Author:Lohrrrr

// This is a main kernel file. It has a LOW level API for other kernel components. The API from this file is not meant for programms writen for this kernel.

#include "api.h"

#ifndef NULL
#define NULL ((void*)0)
#endif


void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

int strcmp_own_hell(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void kmain() {
    
}