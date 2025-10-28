
// Main Kernel
// ver: 0.7
// author: lohrrrr

#include "api.h"
#include "drivers/video/video.h"
#include "drivers/bootparser/parser.h"
#include "drivers/serial/serial.h"

int strcmp_own_hell(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void int_to_str(int num, char* str) {
    int i = 0;
    int is_negative = 0;
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    do {
        str[i++] = num % 10 + '0';
        num /= 10;
    } while (num > 0);
    if (is_negative) str[i++] = '-';
    str[i] = '\0';
    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}


void kmain(unsigned int multiboot_magic, unsigned int multiboot_info_addr) {
    (void)multiboot_magic; // Suppress unused parameter warning
    (void)multiboot_info_addr; // Suppress unused parameter warning

    Video_Init();
    vclear();
    if (get_boot_arg_int("serial") == 1) { serial_init(); serial_puts("Serial started"); }
    if (get_boot_arg_int("rcty") == 1) {
        vprint("RCTY mode enabled.\n");
        rcty_start();
    }
}

