// AHCI SATA Driver API
// Version: 0.1
// Author: Lohrrrr

#ifndef AHCI_H
#define AHCI_H

#include <stdint.h>
#include "../pci/pci.h"

// AHCI Registers
#define AHCI_CAP 0x00
#define AHCI_GHC 0x04
#define AHCI_IS 0x08
#define AHCI_PI 0x0C
#define AHCI_VS 0x10
#define AHCI_CCC_CTL 0x14
#define AHCI_CCC_PORTS 0x18
#define AHCI_EM_LOC 0x1C
#define AHCI_EM_CTL 0x20
#define AHCI_CAP2 0x24
#define AHCI_BOHC 0x28

// Port Registers (offset from port base)
#define AHCI_PxCLB 0x00
#define AHCI_PxCLBU 0x04
#define AHCI_PxFB 0x08
#define AHCI_PxFBU 0x0C
#define AHCI_PxIS 0x10
#define AHCI_PxIE 0x14
#define AHCI_PxCMD 0x18
#define AHCI_PxTFD 0x20
#define AHCI_PxSIG 0x24
#define AHCI_PxSSTS 0x28
#define AHCI_PxSCTL 0x2C
#define AHCI_PxSERR 0x30
#define AHCI_PxSACT 0x34
#define AHCI_PxCI 0x38
#define AHCI_PxSNTF 0x3C
#define AHCI_PxFBS 0x40

// AHCI Structures
typedef struct {
    uint32_t clb;      // Command List Base Address
    uint32_t clbu;     // Command List Base Address Upper 32-bits
    uint32_t fb;       // FIS Base Address
    uint32_t fbu;      // FIS Base Address Upper 32-bits
    uint32_t is;       // Interrupt Status
    uint32_t ie;       // Interrupt Enable
    uint32_t cmd;      // Command and Status
    uint32_t rsv0;     // Reserved
    uint32_t tfd;      // Task File Data
    uint32_t sig;      // Signature
    uint32_t ssts;     // Serial ATA Status
    uint32_t sctl;     // Serial ATA Control
    uint32_t serr;     // Serial ATA Error
    uint32_t sact;     // Serial ATA Active
    uint32_t ci;       // Command Issue
    uint32_t sntf;     // Serial ATA Notification
    uint32_t fbs;      // FIS-based Switching Control
    uint32_t rsv1[11]; // Reserved
    uint32_t vendor[4]; // Vendor Specific
} __attribute__((packed)) ahci_port_t;

typedef struct {
    uint32_t cap;       // Host Capabilities
    uint32_t ghc;       // Global Host Control
    uint32_t is;        // Interrupt Status
    uint32_t pi;        // Ports Implemented
    uint32_t vs;        // Version
    uint32_t ccc_ctl;   // Command Completion Coalescing Control
    uint32_t ccc_ports; // Command Completion Coalescing Ports
    uint32_t em_loc;    // Enclosure Management Location
    uint32_t em_ctl;    // Enclosure Management Control
    uint32_t cap2;      // Host Capabilities Extended
    uint32_t bohc;      // BIOS/OS Handoff Control and Status
    uint8_t rsv[0xA0 - 0x2C]; // Reserved
    uint8_t vendor[0x100 - 0xA0]; // Vendor specific
    ahci_port_t ports[32]; // Port control registers
} __attribute__((packed)) ahci_hba_t;

// Command Header
typedef struct {
    uint8_t cfl : 5;    // Command FIS Length
    uint8_t a : 1;      // ATAPI
    uint8_t w : 1;      // Write
    uint8_t p : 1;      // Prefetchable
    uint8_t r : 1;      // Reset
    uint8_t b : 1;      // BIST
    uint8_t c : 1;      // Clear Busy upon R_OK
    uint8_t rsv0 : 1;   // Reserved
    uint8_t pmp : 4;    // Port Multiplier Port
    uint16_t prdtl;     // Physical Region Descriptor Table Length
    uint32_t prdbc;     // Physical Region Descriptor Byte Count
    uint32_t ctba;      // Command Table Descriptor Base Address
    uint32_t ctbau;     // Command Table Descriptor Base Address Upper 32-bits
    uint32_t rsv1[4];   // Reserved
} __attribute__((packed)) ahci_cmd_header_t;

// Physical Region Descriptor
typedef struct {
    uint32_t dba;       // Data Base Address
    uint32_t dbau;      // Data Base Address Upper 32-bits
    uint32_t rsv0;      // Reserved
    uint32_t dbc : 22;  // Data Byte Count
    uint32_t rsv1 : 9;  // Reserved
    uint32_t i : 1;     // Interrupt on Completion
} __attribute__((packed)) ahci_prdt_t;

// Command Table
typedef struct {
    uint8_t cfis[64];   // Command FIS
    uint8_t acmd[16];   // ATAPI Command
    uint8_t rsv[48];    // Reserved
    ahci_prdt_t prdt[]; // Physical Region Descriptor Table
} __attribute__((packed)) ahci_cmd_table_t;

// FIS Types
#define FIS_TYPE_REG_H2D 0x27
#define FIS_TYPE_REG_D2H 0x34
#define FIS_TYPE_DMA_ACT 0x39
#define FIS_TYPE_DMA_SETUP 0x41
#define FIS_TYPE_DATA 0x46
#define FIS_TYPE_BIST 0x58
#define FIS_TYPE_PIO_SETUP 0x5F
#define FIS_TYPE_DEV_BITS 0xA1

// Register FIS - Host to Device
typedef struct {
    uint8_t fis_type;
    uint8_t pmport : 4;
    uint8_t rsv0 : 3;
    uint8_t c : 1;
    uint8_t command;
    uint8_t featurel;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t featureh;
    uint8_t countl;
    uint8_t counth;
    uint8_t icc;
    uint8_t control;
    uint8_t rsv1[4];
} __attribute__((packed)) fis_reg_h2d_t;

// SATA Functions
int ahci_init(pci_device_t* pci_dev);
int ahci_read_sector(unsigned int port, uint64_t lba, unsigned char* buffer);
int ahci_write_sector(unsigned int port, uint64_t lba, unsigned char* buffer);
int ahci_identify_device(unsigned int port, unsigned char* buffer);
void ahci_port_rebase(ahci_port_t* port, int portno);
int ahci_port_start(ahci_port_t* port);
void ahci_port_stop(ahci_port_t* port);

#endif
