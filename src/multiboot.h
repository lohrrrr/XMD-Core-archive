// Multiboot header definitions
// Version: 0.1
// Author: Lohrrrr

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

// Multiboot info structure
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
} __attribute__((packed)) multiboot_info_t;

// Boot device macros
#define MULTIBOOT_BOOT_DEVICE_DRIVE(boot_device) ((boot_device) & 0xFF)
#define MULTIBOOT_BOOT_DEVICE_PART1(boot_device) (((boot_device) >> 16) & 0xFF)
#define MULTIBOOT_BOOT_DEVICE_PART2(boot_device) (((boot_device) >> 8) & 0xFF)
#define MULTIBOOT_BOOT_DEVICE_PART3(boot_device) (((boot_device) >> 24) & 0xFF)

#endif
