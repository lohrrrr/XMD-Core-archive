// Low Level Kernel API
// Version:0.6
// Author:Lohrrrr

#ifndef KERNEL_H
#define KERNEL_H

#include <stdbool.h>

void kmain(unsigned int multiboot_magic, unsigned int multiboot_info_addr);
int strcmp_own_hell(const char* s1, const char* s2);
void outb(unsigned short port, unsigned char val);
unsigned char inb(unsigned short port);
void rcty_start(void);
void halt_krnl(void);
void int_to_str(int num, char* str);

// ATA functions
void ata_init(void);
int ata_read_sector(unsigned int lba, unsigned char* buffer);
int ata_write_sector(unsigned int lba, unsigned char* buffer);

// FS functions
int fs_mount(unsigned int drive);
int fs_list_root(void);

#endif
