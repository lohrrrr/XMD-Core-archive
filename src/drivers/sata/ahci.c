// AHCI SATA Driver
// Version: 0.1
// Author: Lohrrrr

#include "ahci.h"
#include "../../api.h"
#include "../serial/serial.h"

#define SECTOR_SIZE 512

typedef unsigned int size_t;

static ahci_hba_t* ahci_hba = 0;
static int ahci_ports_count = 0;

// Memory-mapped I/O functions (simplified, assumes identity mapping)
static uint32_t mmio_read32(uintptr_t addr) {
    return *(volatile uint32_t*)addr;
}

static void mmio_write32(uintptr_t addr, uint32_t value) {
    *(volatile uint32_t*)addr = value;
}

// Simple memory allocation (for kernel, use a fixed address)
static void* ahci_alloc(unsigned int size) {
    static uintptr_t alloc_ptr = 0x200000; // Start at 2MB
    void* ptr = (void*)alloc_ptr;
    alloc_ptr += size;
    return ptr;
}

int ahci_init(pci_device_t* pci_dev) {
    serial_puts("[AHCI] Initializing AHCI controller\n");

    // Get ABAR (AHCI Base Address Register)
    uintptr_t abar = pci_dev->bar[5] & 0xFFFFFFF0; // BAR5 for AHCI
    if (abar == 0) {
        serial_puts("[AHCI] Error: Invalid ABAR\n");
        return -1;
    }

    ahci_hba = (ahci_hba_t*)abar;
    serial_puts("[AHCI] ABAR: 0x");
    char abar_str[9];
    int_to_str(abar, abar_str);
    serial_puts(abar_str);
    serial_puts("\n");

    // Enable AHCI mode
    mmio_write32((uintptr_t)&ahci_hba->ghc, 0x80000000); // Set AE bit

    // Get number of ports
    uint32_t pi = mmio_read32((uintptr_t)&ahci_hba->pi);
    ahci_ports_count = 0;
    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) ahci_ports_count++;
    }

    serial_puts("[AHCI] Ports implemented: ");
    char ports_str[3];
    int_to_str(ahci_ports_count, ports_str);
    serial_puts(ports_str);
    serial_puts("\n");

    // Initialize each port
    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) {
            ahci_port_rebase(&ahci_hba->ports[i], i);
            ahci_port_start(&ahci_hba->ports[i]);
        }
    }

    return 0;
}

void ahci_port_rebase(ahci_port_t* port, int portno) {
    ahci_port_stop(port);

    // Command list allocation (1024-byte aligned)
    void* clb = ahci_alloc(1024);
    mmio_write32((uintptr_t)&port->clb, (uintptr_t)clb);
    mmio_write32((uintptr_t)&port->clbu, 0);

    // FIS allocation (256-byte aligned)
    void* fb = ahci_alloc(256);
    mmio_write32((uintptr_t)&port->fb, (uintptr_t)fb);
    mmio_write32((uintptr_t)&port->fbu, 0);

    // Command table allocation
    ahci_cmd_header_t* cmd_header = (ahci_cmd_header_t*)clb;
    for (int i = 0; i < 32; i++) {
        cmd_header[i].prdtl = 8; // 8 PRDT entries
        void* ctba = ahci_alloc(256); // Command table (256 bytes)
        cmd_header[i].ctba = (uintptr_t)ctba;
        cmd_header[i].ctbau = 0;
    }

    // Clear interrupts
    mmio_write32((uintptr_t)&port->is, 0xFFFFFFFF);
}

int ahci_port_start(ahci_port_t* port) {
    // Wait for port to be idle
    int timeout = 1000000;
    while ((mmio_read32((uintptr_t)&port->cmd) & 0xC000) && timeout--);

    if (timeout <= 0) {
        serial_puts("[AHCI] Port start timeout\n");
        return -1;
    }

    // Set FRE and ST bits
    mmio_write32((uintptr_t)&port->cmd, mmio_read32((uintptr_t)&port->cmd) | 0x0010); // FRE
    mmio_write32((uintptr_t)&port->cmd, mmio_read32((uintptr_t)&port->cmd) | 0x0001); // ST

    return 0;
}

void ahci_port_stop(ahci_port_t* port) {
    // Clear ST bit
    mmio_write32((uintptr_t)&port->cmd, mmio_read32((uintptr_t)&port->cmd) & ~0x0001);

    // Wait for FR to clear
    int timeout = 1000000;
    while ((mmio_read32((uintptr_t)&port->cmd) & 0x4000) && timeout--);

    // Clear FRE bit
    mmio_write32((uintptr_t)&port->cmd, mmio_read32((uintptr_t)&port->cmd) & ~0x0010);
}

static int ahci_find_cmdslot(ahci_port_t* port) {
    uint32_t slots = mmio_read32((uintptr_t)&port->sact) | mmio_read32((uintptr_t)&port->ci);
    for (int i = 0; i < 32; i++) {
        if ((slots & (1 << i)) == 0) return i;
    }
    return -1;
}

int ahci_read_sector(unsigned int portno, uint64_t lba, unsigned char* buffer) {
    if (portno >= 32) return -1;
    ahci_port_t* port = &ahci_hba->ports[portno];

    int slot = ahci_find_cmdslot(port);
    if (slot == -1) return -1;

    ahci_cmd_header_t* cmd_header = (ahci_cmd_header_t*)mmio_read32((uintptr_t)&port->clb);
    cmd_header += slot;
    cmd_header->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmd_header->w = 0; // Read
    cmd_header->prdtl = 1; // One PRDT entry

    ahci_cmd_table_t* cmd_table = (ahci_cmd_table_t*)cmd_header->ctba;
    memset(cmd_table, 0, sizeof(ahci_cmd_table_t) + sizeof(ahci_prdt_t));

    // Setup FIS
    fis_reg_h2d_t* fis = (fis_reg_h2d_t*)&cmd_table->cfis;
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c = 1; // Command
    fis->command = 0x25; // READ DMA EXT
    fis->lba0 = lba & 0xFF;
    fis->lba1 = (lba >> 8) & 0xFF;
    fis->lba2 = (lba >> 16) & 0xFF;
    fis->lba3 = (lba >> 24) & 0xFF;
    fis->lba4 = (lba >> 32) & 0xFF;
    fis->lba5 = (lba >> 40) & 0xFF;
    fis->device = 1 << 6; // LBA mode
    fis->countl = 1; // One sector

    // Setup PRDT
    cmd_table->prdt[0].dba = (uintptr_t)buffer;
    cmd_table->prdt[0].dbc = SECTOR_SIZE - 1;
    cmd_table->prdt[0].i = 1; // Interrupt on completion

    // Issue command
    mmio_write32((uintptr_t)&port->ci, 1 << slot);

    // Wait for completion
    int timeout = 1000000;
    while ((mmio_read32((uintptr_t)&port->ci) & (1 << slot)) && timeout--);

    if (timeout <= 0) {
        serial_puts("[AHCI] Read timeout\n");
        return -1;
    }

    // Check for errors
    if (mmio_read32((uintptr_t)&port->is) & 0x40000000) {
        serial_puts("[AHCI] Read error\n");
        return -1;
    }

    return 0;
}

int ahci_write_sector(unsigned int portno, uint64_t lba, unsigned char* buffer) {
    if (portno >= 32) return -1;
    ahci_port_t* port = &ahci_hba->ports[portno];

    int slot = ahci_find_cmdslot(port);
    if (slot == -1) return -1;

    ahci_cmd_header_t* cmd_header = (ahci_cmd_header_t*)mmio_read32((uintptr_t)&port->clb);
    cmd_header += slot;
    cmd_header->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmd_header->w = 1; // Write
    cmd_header->prdtl = 1;

    ahci_cmd_table_t* cmd_table = (ahci_cmd_table_t*)cmd_header->ctba;
    memset(cmd_table, 0, sizeof(ahci_cmd_table_t) + sizeof(ahci_prdt_t));

    // Setup FIS
    fis_reg_h2d_t* fis = (fis_reg_h2d_t*)&cmd_table->cfis;
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c = 1;
    fis->command = 0x35; // WRITE DMA EXT
    fis->lba0 = lba & 0xFF;
    fis->lba1 = (lba >> 8) & 0xFF;
    fis->lba2 = (lba >> 16) & 0xFF;
    fis->lba3 = (lba >> 24) & 0xFF;
    fis->lba4 = (lba >> 32) & 0xFF;
    fis->lba5 = (lba >> 40) & 0xFF;
    fis->device = 1 << 6;
    fis->countl = 1;

    // Setup PRDT
    cmd_table->prdt[0].dba = (uintptr_t)buffer;
    cmd_table->prdt[0].dbc = SECTOR_SIZE - 1;
    cmd_table->prdt[0].i = 1;

    // Issue command
    mmio_write32((uintptr_t)&port->ci, 1 << slot);

    // Wait for completion
    int timeout = 1000000;
    while ((mmio_read32((uintptr_t)&port->ci) & (1 << slot)) && timeout--);

    if (timeout <= 0) {
        serial_puts("[AHCI] Write timeout\n");
        return -1;
    }

    if (mmio_read32((uintptr_t)&port->is) & 0x40000000) {
        serial_puts("[AHCI] Write error\n");
        return -1;
    }

    return 0;
}

int ahci_identify_device(unsigned int portno, unsigned char* buffer) {
    if (portno >= 32) return -1;
    ahci_port_t* port = &ahci_hba->ports[portno];

    int slot = ahci_find_cmdslot(port);
    if (slot == -1) return -1;

    ahci_cmd_header_t* cmd_header = (ahci_cmd_header_t*)mmio_read32((uintptr_t)&port->clb);
    cmd_header += slot;
    cmd_header->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmd_header->w = 0;
    cmd_header->prdtl = 1;

    ahci_cmd_table_t* cmd_table = (ahci_cmd_table_t*)cmd_header->ctba;
    memset(cmd_table, 0, sizeof(ahci_cmd_table_t) + sizeof(ahci_prdt_t));

    // Setup FIS for IDENTIFY
    fis_reg_h2d_t* fis = (fis_reg_h2d_t*)&cmd_table->cfis;
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c = 1;
    fis->command = 0xEC; // IDENTIFY DEVICE
    fis->device = 0;
    fis->countl = 1;

    // Setup PRDT
    cmd_table->prdt[0].dba = (uintptr_t)buffer;
    cmd_table->prdt[0].dbc = SECTOR_SIZE - 1;
    cmd_table->prdt[0].i = 1;

    // Issue command
    mmio_write32((uintptr_t)&port->ci, 1 << slot);

    // Wait for completion
    int timeout = 1000000;
    while ((mmio_read32((uintptr_t)&port->ci) & (1 << slot)) && timeout--);

    if (timeout <= 0) {
        serial_puts("[AHCI] Identify timeout\n");
        return -1;
    }

    if (mmio_read32((uintptr_t)&port->is) & 0x40000000) {
        serial_puts("[AHCI] Identify error\n");
        return -1;
    }

    return 0;
}
