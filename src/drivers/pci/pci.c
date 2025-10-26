// PCI Driver
// Version: 0.1
// Author: Lohrrrr

#include "pci.h"
#include "../../api.h"
#include "../serial/serial.h"

// PCI Configuration Space Access
uint32_t pci_read_config_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outb(PCI_CONFIG_ADDRESS, address);
    return inb(PCI_CONFIG_DATA) | (inb(PCI_CONFIG_DATA) << 8) | (inb(PCI_CONFIG_DATA) << 16) | (inb(PCI_CONFIG_DATA) << 24);
}

uint16_t pci_read_config_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t dword = pci_read_config_dword(bus, device, function, offset & 0xFC);
    return (dword >> ((offset & 3) * 8)) & 0xFFFF;
}

uint8_t pci_read_config_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t dword = pci_read_config_dword(bus, device, function, offset & 0xFC);
    return (dword >> ((offset & 3) * 8)) & 0xFF;
}

void pci_write_config_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outb(PCI_CONFIG_ADDRESS, address);
    outb(PCI_CONFIG_DATA, value & 0xFF);
    outb(PCI_CONFIG_DATA, (value >> 8) & 0xFF);
    outb(PCI_CONFIG_DATA, (value >> 16) & 0xFF);
    outb(PCI_CONFIG_DATA, (value >> 24) & 0xFF);
}

void pci_write_config_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value) {
    uint32_t dword = pci_read_config_dword(bus, device, function, offset & 0xFC);
    uint8_t shift = (offset & 3) * 8;
    dword = (dword & ~(0xFFFF << shift)) | (value << shift);
    pci_write_config_dword(bus, device, function, offset & 0xFC, dword);
}

void pci_write_config_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value) {
    uint32_t dword = pci_read_config_dword(bus, device, function, offset & 0xFC);
    uint8_t shift = (offset & 3) * 8;
    dword = (dword & ~(0xFF << shift)) | (value << shift);
    pci_write_config_dword(bus, device, function, offset & 0xFC, dword);
}

void pci_scan_bus(void) {
    serial_puts("[PCI] Scanning PCI bus 0\n");
    for (uint8_t device = 0; device < 32; device++) {
        for (uint8_t function = 0; function < 8; function++) {
            uint16_t vendor_id = pci_read_config_word(0, device, function, 0);
            if (vendor_id == 0xFFFF) continue; // No device

            uint16_t device_id = pci_read_config_word(0, device, function, 2);
            uint8_t class_code = pci_read_config_byte(0, device, function, 0x0B);
            uint8_t subclass = pci_read_config_byte(0, device, function, 0x0A);
            uint8_t prog_if = pci_read_config_byte(0, device, function, 0x09);

            serial_puts("[PCI] Found device: bus=0, dev=");
            char dev_str[3];
            int_to_str(device, dev_str);
            serial_puts(dev_str);
            serial_puts(", func=");
            char func_str[2];
            int_to_str(function, func_str);
            serial_puts(func_str);
            serial_puts(", vendor=0x");
            char vendor_hex[5];
            int_to_str(vendor_id, vendor_hex);
            serial_puts(vendor_hex);
            serial_puts(", device=0x");
            char device_hex[5];
            int_to_str(device_id, device_hex);
            serial_puts(device_hex);
            serial_puts(", class=");
            char class_str[3];
            int_to_str(class_code, class_str);
            serial_puts(class_str);
            serial_puts(", subclass=");
            char subclass_str[3];
            int_to_str(subclass, subclass_str);
            serial_puts(subclass_str);
            serial_puts("\n");

            // Check if it's a SATA controller (class 1, subclass 6)
            if (class_code == 0x01 && subclass == 0x06) {
                serial_puts("[PCI] SATA controller found!\n");
            }

            // Check for multi-function device
            if (function == 0) {
                uint8_t header_type = pci_read_config_byte(0, device, function, 0x0E);
                if (!(header_type & 0x80)) break; // Not multi-function
            }
        }
    }
}

int pci_find_device(uint16_t vendor_id, uint16_t device_id, pci_device_t* device) {
    for (uint8_t dev = 0; dev < 32; dev++) {
        for (uint8_t func = 0; func < 8; func++) {
            uint16_t vid = pci_read_config_word(0, dev, func, 0);
            uint16_t did = pci_read_config_word(0, dev, func, 2);
            if (vid == vendor_id && did == device_id) {
                // Fill device structure
                device->vendor_id = vid;
                device->device_id = did;
                device->command = pci_read_config_word(0, dev, func, 4);
                device->status = pci_read_config_word(0, dev, func, 6);
                device->revision_id = pci_read_config_byte(0, dev, func, 8);
                device->prog_if = pci_read_config_byte(0, dev, func, 9);
                device->subclass = pci_read_config_byte(0, dev, func, 0x0A);
                device->class_code = pci_read_config_byte(0, dev, func, 0x0B);
                device->cache_line_size = pci_read_config_byte(0, dev, func, 0x0C);
                device->latency_timer = pci_read_config_byte(0, dev, func, 0x0D);
                device->header_type = pci_read_config_byte(0, dev, func, 0x0E);
                device->bist = pci_read_config_byte(0, dev, func, 0x0F);
                for (int i = 0; i < 6; i++) {
                    device->bar[i] = pci_read_config_dword(0, dev, func, 0x10 + i*4);
                }
                device->cardbus_cis_ptr = pci_read_config_dword(0, dev, func, 0x28);
                device->subsystem_vendor_id = pci_read_config_word(0, dev, func, 0x2C);
                device->subsystem_id = pci_read_config_word(0, dev, func, 0x2E);
                device->expansion_rom_base_addr = pci_read_config_dword(0, dev, func, 0x30);
                device->capabilities_ptr = pci_read_config_byte(0, dev, func, 0x34);
                device->interrupt_line = pci_read_config_byte(0, dev, func, 0x3C);
                device->interrupt_pin = pci_read_config_byte(0, dev, func, 0x3D);
                device->min_grant = pci_read_config_byte(0, dev, func, 0x3E);
                device->max_latency = pci_read_config_byte(0, dev, func, 0x3F);
                return 0; // Found
            }
            if (func == 0) {
                uint8_t header_type = pci_read_config_byte(0, dev, func, 0x0E);
                if (!(header_type & 0x80)) break;
            }
        }
    }
    return -1; // Not found
}

int pci_find_class(uint8_t class_code, uint8_t subclass, pci_device_t* device) {
    for (uint8_t dev = 0; dev < 32; dev++) {
        for (uint8_t func = 0; func < 8; func++) {
            uint8_t cls = pci_read_config_byte(0, dev, func, 0x0B);
            uint8_t sub = pci_read_config_byte(0, dev, func, 0x0A);
            if (cls == class_code && sub == subclass) {
                // Fill device structure (similar to above)
                device->vendor_id = pci_read_config_word(0, dev, func, 0);
                device->device_id = pci_read_config_word(0, dev, func, 2);
                device->command = pci_read_config_word(0, dev, func, 4);
                device->status = pci_read_config_word(0, dev, func, 6);
                device->revision_id = pci_read_config_byte(0, dev, func, 8);
                device->prog_if = pci_read_config_byte(0, dev, func, 9);
                device->subclass = sub;
                device->class_code = cls;
                device->cache_line_size = pci_read_config_byte(0, dev, func, 0x0C);
                device->latency_timer = pci_read_config_byte(0, dev, func, 0x0D);
                device->header_type = pci_read_config_byte(0, dev, func, 0x0E);
                device->bist = pci_read_config_byte(0, dev, func, 0x0F);
                for (int i = 0; i < 6; i++) {
                    device->bar[i] = pci_read_config_dword(0, dev, func, 0x10 + i*4);
                }
                device->cardbus_cis_ptr = pci_read_config_dword(0, dev, func, 0x28);
                device->subsystem_vendor_id = pci_read_config_word(0, dev, func, 0x2C);
                device->subsystem_id = pci_read_config_word(0, dev, func, 0x2E);
                device->expansion_rom_base_addr = pci_read_config_dword(0, dev, func, 0x30);
                device->capabilities_ptr = pci_read_config_byte(0, dev, func, 0x34);
                device->interrupt_line = pci_read_config_byte(0, dev, func, 0x3C);
                device->interrupt_pin = pci_read_config_byte(0, dev, func, 0x3D);
                device->min_grant = pci_read_config_byte(0, dev, func, 0x3E);
                device->max_latency = pci_read_config_byte(0, dev, func, 0x3F);
                return 0; // Found
            }
            if (func == 0) {
                uint8_t header_type = pci_read_config_byte(0, dev, func, 0x0E);
                if (!(header_type & 0x80)) break;
            }
        }
    }
    return -1; // Not found
}

void pci_init(void) {
    serial_puts("[PCI] Initializing PCI subsystem\n");
    pci_scan_bus();
}
