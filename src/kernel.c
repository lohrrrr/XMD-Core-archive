#include "kernel.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

// Utility functions

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

// Boot configuration

// #define MAX_BOOT_ARGS 128

// struct BootConfig {
//     int video;
//     int shell;
// };

// struct BootConfig bootcfg;

// void parse_boot_params(const char* cmdline) {
//     bootcfg.video = 1;
//     bootcfg.shell = 1;

//     const char* p = cmdline;
//     char word[32];
//     int i = 0;

//     while (*p) {
//         if (*p == ' ' || *p == '\0') {
//             if (i > 0) {
//                 word[i] = 0;
//                 if (strcmp_own_hell(word, "video=0") == 0) {
//                     bootcfg.video = 0;
//                 }
//                 if (strcmp_own_hell(word, "shell=1") == 0) {
//                     bootcfg.shell = 1;
//                 }
//                 if (strcmp_own_hell(word, "shell=0") == 0) {
//                     bootcfg.shell = 0;
//                 }
//             }
//             i = 0;
//             if (*p == '\0') break;
//         } else {
//             if (i < 31) word[i++] = *p;
//         }
//         p++;
//     }
// }


// // Global variables

// const char* grub_cmdline_global;

// Main kernel function

void kmain() {
    
}