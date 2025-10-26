// Main Kernel
// Version:0.6 (full rewrite)
// Author:Lohrrrr

// This is a main kernel file. It has a LOW level API for other kernel components. The API from this file is not meant for programms writen for this kernel.

#include "api.h"
#include "drivers/video/video.h"
#include "drivers/bootparser/parser.h"
#include "drivers/serial/serial.h"
#include "drivers/fs/fs.h"
#include "drivers/pci/pci.h"
#include "drivers/sata/ahci.h"
#include "multiboot.h"


void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

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

void halt_krnl() {
    while (1) {
        __asm__ volatile ("hlt");
    }
}

void kmain(unsigned int multiboot_magic, unsigned int multiboot_info_addr) {
    serial_init();
    Video_Init();
    vclear();

    // Initialize PCI and SATA
    pci_init();
    pci_device_t sata_dev;
    if (pci_find_class(0x01, 0x06, &sata_dev) == 0) {
        ahci_init(&sata_dev);
    }

    multiboot_info_t* mbi = (multiboot_info_t*)multiboot_info_addr;
    if (mbi->flags & (1 << 1)) {
        unsigned int boot_device = mbi->boot_device;
        unsigned int drive = MULTIBOOT_BOOT_DEVICE_DRIVE(boot_device);
        fs_set_boot_drive(drive);
    }

    if (get_boot_arg_int("rcty") == 1) {
        vprint("RCTY mode enabled.\n");
        rcty_start();
    } else {
    }
}

