// ATA/IDE Driver
// Version: 0.1
// Author: Lohrrrr

#include "ata.h"
#include "../../api.h"
#include "../serial/serial.h"



void ata_outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void ata_init(void) {
    // Basic initialization - assume primary master drive
    // In a full implementation, detect drives, etc.
}

static void ata_wait_ready(void) {
    int timeout = 1000000;  // Timeout to prevent infinite loop
    while ((inb(ATA_PRIMARY_IO + ATA_STATUS) & ATA_STATUS_BUSY) && timeout--);
}

static void ata_wait_drq(void) {
    int timeout = 1000000;  // Timeout to prevent infinite loop
    while (!(inb(ATA_PRIMARY_IO + ATA_STATUS) & ATA_STATUS_DRQ) && timeout--);
}

int ata_read_sector(unsigned int lba, unsigned char* buffer) {
    ata_wait_ready();

    // Send parameters
    ata_outb(ATA_PRIMARY_IO + ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));  // Master drive
    ata_outb(ATA_PRIMARY_IO + ATA_SECTOR_COUNT, 1);
    ata_outb(ATA_PRIMARY_IO + ATA_SECTOR_NUMBER, lba & 0xFF);
    ata_outb(ATA_PRIMARY_IO + ATA_CYLINDER_LOW, (lba >> 8) & 0xFF);
    ata_outb(ATA_PRIMARY_IO + ATA_CYLINDER_HIGH, (lba >> 16) & 0xFF);
    ata_outb(ATA_PRIMARY_IO + ATA_COMMAND, ATA_CMD_READ_SECTORS);

    ata_wait_ready();
    if (inb(ATA_PRIMARY_IO + ATA_STATUS) & ATA_STATUS_ERR) {
        return -1;  // Error
    }

    ata_wait_drq();

    // Read 256 words (512 bytes)
    for (int i = 0; i < 256; i++) {
        unsigned short word = inb(ATA_PRIMARY_IO + ATA_DATA) | (inb(ATA_PRIMARY_IO + ATA_DATA) << 8);
        buffer[i*2] = word & 0xFF;
        buffer[i*2 + 1] = (word >> 8) & 0xFF;
    }

    return 0;  // Success
}

int ata_write_sector(unsigned int lba, unsigned char* buffer) {
    ata_wait_ready();

    // Send parameters
    ata_outb(ATA_PRIMARY_IO + ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    ata_outb(ATA_PRIMARY_IO + ATA_SECTOR_COUNT, 1);
    ata_outb(ATA_PRIMARY_IO + ATA_SECTOR_NUMBER, lba & 0xFF);
    ata_outb(ATA_PRIMARY_IO + ATA_CYLINDER_LOW, (lba >> 8) & 0xFF);
    ata_outb(ATA_PRIMARY_IO + ATA_CYLINDER_HIGH, (lba >> 16) & 0xFF);
    ata_outb(ATA_PRIMARY_IO + ATA_COMMAND, ATA_CMD_WRITE_SECTORS);

    ata_wait_ready();
    if (inb(ATA_PRIMARY_IO + ATA_STATUS) & ATA_STATUS_ERR) {
        return -1;
    }

    ata_wait_drq();

    // Write 256 words
    for (int i = 0; i < 256; i++) {
        unsigned short word = buffer[i*2] | (buffer[i*2 + 1] << 8);
        ata_outb(ATA_PRIMARY_IO + ATA_DATA, word & 0xFF);
        ata_outb(ATA_PRIMARY_IO + ATA_DATA, (word >> 8) & 0xFF);
    }

    return 0;
}

int ata_read_sector_drive(unsigned int drive, unsigned int lba, unsigned char* buffer) {
    serial_puts("[ATA] Reading sector ");
    char lba_str[16];
    int_to_str(lba, lba_str);
    serial_puts(lba_str);
    serial_puts(" from drive ");
    char drive_str[2] = {'0' + drive, '\0'};
    serial_puts(drive_str);
    serial_puts("\n");

    // Map drive numbers to ATA channels and master/slave
    // drive 0-1: channel 0 (primary), drive 2-3: channel 1 (secondary)
    // drive 4-5: channel 2, drive 6-7: channel 3, etc.
    unsigned int channel = drive / 2;
    unsigned int master_slave = drive % 2;

    unsigned short io_base;
    switch (channel) {
        case 0: io_base = ATA_CHANNEL0_IO; break;
        case 1: io_base = ATA_CHANNEL1_IO; break;
        case 2: io_base = ATA_CHANNEL2_IO; break;
        case 3: io_base = ATA_CHANNEL3_IO; break;
        default: return -1; // Unsupported channel
    }

    unsigned char drive_select = 0xE0 | ((lba >> 24) & 0x0F);
    if (master_slave == 1) drive_select |= 0x10;  // Slave drive

    ata_wait_ready();

    // Send parameters
    ata_outb(io_base + ATA_DRIVE_HEAD, drive_select);
    ata_outb(io_base + ATA_SECTOR_COUNT, 1);
    ata_outb(io_base + ATA_SECTOR_NUMBER, lba & 0xFF);
    ata_outb(io_base + ATA_CYLINDER_LOW, (lba >> 8) & 0xFF);
    ata_outb(io_base + ATA_CYLINDER_HIGH, (lba >> 16) & 0xFF);
    ata_outb(io_base + ATA_COMMAND, ATA_CMD_READ_SECTORS);

    ata_wait_ready();
    if (inb(io_base + ATA_STATUS) & ATA_STATUS_ERR) {
        serial_puts("[ATA] Read error\n");
        return -1;  // Error
    }

    ata_wait_drq();

    // Read 256 words (512 bytes)
    for (int i = 0; i < 256; i++) {
        unsigned short word = inb(io_base + ATA_DATA) | (inb(io_base + ATA_DATA) << 8);
        buffer[i*2] = word & 0xFF;
        buffer[i*2 + 1] = (word >> 8) & 0xFF;
    }

    serial_puts("[ATA] Read successful\n");
    return 0;  // Success
}

int ata_write_sector_drive(unsigned int drive, unsigned int lba, unsigned char* buffer) {
    serial_puts("[ATA] Writing sector ");
    char lba_str[16];
    int_to_str(lba, lba_str);
    serial_puts(lba_str);
    serial_puts(" to drive ");
    char drive_str[2] = {'0' + drive, '\0'};
    serial_puts(drive_str);
    serial_puts("\n");

    // Map drive numbers to ATA channels and master/slave
    // drive 0-1: channel 0 (primary), drive 2-3: channel 1 (secondary)
    // drive 4-5: channel 2, drive 6-7: channel 3, etc.
    unsigned int channel = drive / 2;
    unsigned int master_slave = drive % 2;

    unsigned short io_base;
    switch (channel) {
        case 0: io_base = ATA_CHANNEL0_IO; break;
        case 1: io_base = ATA_CHANNEL1_IO; break;
        case 2: io_base = ATA_CHANNEL2_IO; break;
        case 3: io_base = ATA_CHANNEL3_IO; break;
        default: return -1; // Unsupported channel
    }

    unsigned char drive_select = 0xE0 | ((lba >> 24) & 0x0F);
    if (master_slave == 1) drive_select |= 0x10;  // Slave drive

    ata_wait_ready();

    // Send parameters
    ata_outb(io_base + ATA_DRIVE_HEAD, drive_select);
    ata_outb(io_base + ATA_SECTOR_COUNT, 1);
    ata_outb(io_base + ATA_SECTOR_NUMBER, lba & 0xFF);
    ata_outb(io_base + ATA_CYLINDER_LOW, (lba >> 8) & 0xFF);
    ata_outb(io_base + ATA_CYLINDER_HIGH, (lba >> 16) & 0xFF);
    ata_outb(io_base + ATA_COMMAND, ATA_CMD_WRITE_SECTORS);

    ata_wait_ready();
    if (inb(io_base + ATA_STATUS) & ATA_STATUS_ERR) {
        serial_puts("[ATA] Write error\n");
        return -1;
    }

    ata_wait_drq();

    // Write 256 words
    for (int i = 0; i < 256; i++) {
        unsigned short word = buffer[i*2] | (buffer[i*2 + 1] << 8);
        ata_outb(io_base + ATA_DATA, word & 0xFF);
        ata_outb(io_base + ATA_DATA, (word >> 8) & 0xFF);
    }

    serial_puts("[ATA] Write successful\n");
    return 0;
}
