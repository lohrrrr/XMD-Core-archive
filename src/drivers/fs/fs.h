// Filesystem Driver API
// Version: 0.2
// Author: Lohrrrr

#ifndef FS_H
#define FS_H

#include <stdbool.h>

#define SECTOR_SIZE 512

// FAT Boot Sector structure (common for FAT16/FAT32)
typedef struct {
    unsigned char jump[3];
    char oem_name[8];
    unsigned short bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char num_fats;
    unsigned short root_entries;
    unsigned short total_sectors_16;
    unsigned char media_descriptor;
    unsigned short sectors_per_fat_16;
    unsigned short sectors_per_track;
    unsigned short num_heads;
    unsigned int hidden_sectors;
    unsigned int total_sectors_32;
    unsigned int sectors_per_fat_32;
    unsigned short flags;
    unsigned short version;
    unsigned int root_cluster;
    unsigned short fs_info_sector;
    unsigned short backup_boot_sector;
    unsigned char reserved[12];
    unsigned char drive_number;
    unsigned char reserved1;
    unsigned char boot_signature;
    unsigned int volume_id;
    char volume_label[11];
    char fs_type[8];
} __attribute__((packed)) FATBootSector;

// Directory Entry structure
typedef struct {
    char name[11];
    unsigned char attributes;
    unsigned char reserved;
    unsigned char creation_time_tenths;
    unsigned short creation_time;
    unsigned short creation_date;
    unsigned short last_access_date;
    unsigned short first_cluster_high;
    unsigned short last_mod_time;
    unsigned short last_mod_date;
    unsigned short first_cluster_low;
    unsigned int file_size;
} __attribute__((packed)) FATDirEntry;

// FAT16 Filesystem Structure
typedef struct {
    FATBootSector boot_sector;
    unsigned int drive_number;
    unsigned int fat_start;
    unsigned int data_start;
    unsigned int root_dir_start;
    unsigned int root_dir_sectors;
} FAT16FS;

// FAT32 Filesystem Structure
typedef struct {
    FATBootSector boot_sector;
    unsigned int drive_number;
    unsigned int fat_start;
    unsigned int data_start;
    unsigned int root_dir_start;
} FAT32FS;

// Generic Filesystem Types
typedef enum {
    FS_NONE = 0,
    FS_FAT16,
    FS_FAT32,
    FS_EXT2,
    FS_NTFS
} FSType;

// SATA Drive Numbers start from 8 (ATA uses 0-7)
#define ATA_DRIVE_COUNT 3
#define SATA_DRIVE_OFFSET (ATA_DRIVE_COUNT + 1)  // +1 to account for drive 0

// Generic Filesystem Structure
typedef struct {
    FSType type;
    unsigned int drive_number;
    union {
        FAT16FS fat16;
        FAT32FS fat32;
        // Add other FS structures here
    } fs;
} GenericFS;

extern GenericFS* mounted_fs;

int fs_mount(unsigned int drive);
int fs_list_root(void);
int fs_detect_usb_drives(void);
void fs_set_boot_drive(unsigned int drive);
void fs_list_drives(void);
int fs_format(unsigned int drive, const char* fs_type);
int fs_detect_sata_drives(void);

#endif
