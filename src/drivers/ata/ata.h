// ATA/IDE Driver API
// Version: 0.1
// Author: Lohrrrr

#ifndef ATA_H
#define ATA_H

#define ATA_CHANNEL0_IO 0x1F0
#define ATA_CHANNEL0_CONTROL 0x3F6
#define ATA_CHANNEL1_IO 0x170
#define ATA_CHANNEL1_CONTROL 0x376
#define ATA_CHANNEL2_IO 0x1E0
#define ATA_CHANNEL2_CONTROL 0x3E6
#define ATA_CHANNEL3_IO 0x160
#define ATA_CHANNEL3_CONTROL 0x366

#define ATA_PRIMARY_IO ATA_CHANNEL0_IO
#define ATA_PRIMARY_CONTROL ATA_CHANNEL0_CONTROL
#define ATA_SECONDARY_IO ATA_CHANNEL1_IO
#define ATA_SECONDARY_CONTROL ATA_CHANNEL1_CONTROL

#define ATA_DATA 0x00
#define ATA_ERROR 0x01
#define ATA_SECTOR_COUNT 0x02
#define ATA_SECTOR_NUMBER 0x03
#define ATA_CYLINDER_LOW 0x04
#define ATA_CYLINDER_HIGH 0x05
#define ATA_DRIVE_HEAD 0x06
#define ATA_STATUS 0x07
#define ATA_COMMAND 0x07

#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_STATUS_BUSY 0x80
#define ATA_STATUS_READY 0x40
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_ERR 0x01

void ata_init(void);
int ata_read_sector(unsigned int lba, unsigned char* buffer);
int ata_write_sector(unsigned int lba, unsigned char* buffer);
int ata_read_sector_drive(unsigned int drive, unsigned int lba, unsigned char* buffer);
int ata_write_sector_drive(unsigned int drive, unsigned int lba, unsigned char* buffer);

#endif
