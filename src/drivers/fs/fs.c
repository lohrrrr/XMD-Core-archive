// Filesystem Driver
// Version: 0.2
// Author: Lohrrrr


#include "fs.h"
#include "../ata/ata.h"
#include "../sata/ahci.h"
#include "../video/video.h"
#include "../../api.h"
#include "../serial/serial.h"

// Simple string functions for kernel
typedef unsigned int size_t;

void* memcpy(void* dest, const void* src, size_t n) {
    char* d = dest;
    const char* s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void* memset(void* s, int c, size_t n) {
    char* p = s;
    while (n--) *p++ = c;
    return s;
}

GenericFS* mounted_fs = 0;
static unsigned int boot_drive = 0; // Store the boot drive number

static unsigned int cluster_to_lba_fat32(unsigned int cluster) {
    return mounted_fs->fs.fat32.data_start + (cluster - 2) * mounted_fs->fs.fat32.boot_sector.sectors_per_cluster;
}

static unsigned int cluster_to_lba_fat16(unsigned int cluster) {
    return mounted_fs->fs.fat16.data_start + (cluster - 2) * mounted_fs->fs.fat16.boot_sector.sectors_per_cluster;
}

static unsigned int get_next_cluster_fat32(unsigned int cluster) {
    unsigned int fat_sector = mounted_fs->fs.fat32.fat_start + (cluster * 4) / SECTOR_SIZE;
    unsigned char buffer[SECTOR_SIZE];
    int read_result = (mounted_fs->drive_number >= SATA_DRIVE_OFFSET) ?
        ahci_read_sector(mounted_fs->drive_number - SATA_DRIVE_OFFSET, fat_sector, buffer) :
        ata_read_sector_drive(mounted_fs->drive_number, fat_sector, buffer);
    if (read_result != 0) return 0xFFFFFFFF;

    unsigned int offset = (cluster * 4) % SECTOR_SIZE;
    unsigned int next_cluster = *(unsigned int*)&buffer[offset] & 0x0FFFFFFF;
    if (next_cluster >= 0x0FFFFFF8) return 0xFFFFFFFF;
    return next_cluster;
}

static unsigned int get_next_cluster_fat16(unsigned int cluster) {
    unsigned int fat_sector = mounted_fs->fs.fat16.fat_start + (cluster * 2) / SECTOR_SIZE;
    unsigned char buffer[SECTOR_SIZE];
    int read_result = (mounted_fs->drive_number >= SATA_DRIVE_OFFSET) ?
        ahci_read_sector(mounted_fs->drive_number - SATA_DRIVE_OFFSET, fat_sector, buffer) :
        ata_read_sector_drive(mounted_fs->drive_number, fat_sector, buffer);
    if (read_result != 0) return 0xFFFF;

    unsigned int offset = (cluster * 2) % SECTOR_SIZE;
    unsigned short next_cluster = *(unsigned short*)&buffer[offset];
    if (next_cluster >= 0xFFF8) return 0xFFFF;
    return next_cluster;
}

int fs_mount(unsigned int drive) {
    serial_puts("[FS] Mounting drive ");
    char drive_str[2] = {'0' + drive, '\0'};
    serial_puts(drive_str);
    serial_puts("\n");

    if (mounted_fs) {
        vprint("Error: Filesystem already mounted\n");
        serial_puts("[FS] Error: Filesystem already mounted\n");
        return -1;
    }

    mounted_fs = (GenericFS*)0x100000;
    mounted_fs->drive_number = drive;

    unsigned char buffer[SECTOR_SIZE];
    int read_result;
    if (drive >= SATA_DRIVE_OFFSET) {
        // SATA drive
        read_result = ahci_read_sector(drive - SATA_DRIVE_OFFSET, 0, buffer);
    } else {
        // ATA drive
        read_result = ata_read_sector_drive(drive, 0, buffer);
    }
    if (read_result != 0) {
        vprint("Error: Failed to read boot sector from drive\n");
        serial_puts("[FS] Error: Failed to read boot sector\n");
        mounted_fs = 0;
        return -1;
    }

    serial_puts("[FS] Boot sector read OK\n");

    FATBootSector* boot_sector = (FATBootSector*)buffer;

    // Check for supported filesystems based on OSDev wiki
    // For now, only support FAT32 and FAT16
    int is_fat32 = 0;
    int is_fat16 = 0;

    // Check FAT32 signature (bytes per sector, etc.)
    if (boot_sector->bytes_per_sector == 512 &&
        boot_sector->sectors_per_cluster > 0 &&
        boot_sector->reserved_sectors > 0 &&
        boot_sector->num_fats > 0 &&
        boot_sector->root_entries == 0 &&  // FAT32 has no root entries
        boot_sector->total_sectors_16 == 0 &&
        boot_sector->sectors_per_fat_16 == 0 &&
        boot_sector->sectors_per_fat_32 > 0) {
        is_fat32 = 1;
    }

    // Check FAT16 signature
    if (boot_sector->bytes_per_sector == 512 &&
        boot_sector->sectors_per_cluster > 0 &&
        boot_sector->reserved_sectors > 0 &&
        boot_sector->num_fats > 0 &&
        boot_sector->root_entries > 0 &&  // FAT16 has root entries
        boot_sector->sectors_per_fat_16 > 0) {
        is_fat16 = 1;
    }

    if (is_fat32) {
        serial_puts("[FS] Detected FAT32 filesystem\n");
        mounted_fs->type = FS_FAT32;
        for (int i = 0; i < sizeof(FATBootSector); i++) {
            ((unsigned char*)&mounted_fs->fs.fat32.boot_sector)[i] = buffer[i];
        }
        mounted_fs->fs.fat32.fat_start = mounted_fs->fs.fat32.boot_sector.reserved_sectors;
        mounted_fs->fs.fat32.data_start = mounted_fs->fs.fat32.fat_start + mounted_fs->fs.fat32.boot_sector.num_fats * mounted_fs->fs.fat32.boot_sector.sectors_per_fat_32;
        mounted_fs->fs.fat32.root_dir_start = cluster_to_lba_fat32(mounted_fs->fs.fat32.boot_sector.root_cluster);
    } else if (is_fat16) {
        serial_puts("[FS] Detected FAT16 filesystem\n");
        mounted_fs->type = FS_FAT16;
        for (int i = 0; i < sizeof(FATBootSector); i++) {
            ((unsigned char*)&mounted_fs->fs.fat16.boot_sector)[i] = buffer[i];
        }
        mounted_fs->fs.fat16.fat_start = mounted_fs->fs.fat16.boot_sector.reserved_sectors;
        mounted_fs->fs.fat16.root_dir_sectors = ((mounted_fs->fs.fat16.boot_sector.root_entries * 32) + (SECTOR_SIZE - 1)) / SECTOR_SIZE;
        mounted_fs->fs.fat16.data_start = mounted_fs->fs.fat16.fat_start + mounted_fs->fs.fat16.boot_sector.num_fats * mounted_fs->fs.fat16.boot_sector.sectors_per_fat_16 + mounted_fs->fs.fat16.root_dir_sectors;
        mounted_fs->fs.fat16.root_dir_start = mounted_fs->fs.fat16.fat_start + mounted_fs->fs.fat16.boot_sector.num_fats * mounted_fs->fs.fat16.boot_sector.sectors_per_fat_16;
    } else {
        // Create null-terminated string for printing
        char fs_type_str[9];
        memcpy(fs_type_str, boot_sector->fs_type, 8);
        fs_type_str[8] = '\0';

        serial_puts("[FS] Unknown filesystem type: ");
        serial_puts(fs_type_str);
        serial_puts("\n");

        // Debug: print hex values
        serial_puts("[FS] fs_type bytes: ");
        for (int i = 0; i < 8; i++) {
            char hex[3];
            int_to_str((unsigned char)boot_sector->fs_type[i], hex);
            if (hex[1] == '\0') {
                hex[1] = ' ';
                hex[2] = '\0';
            }
            serial_puts(hex);
            serial_puts(" ");
        }
        serial_puts("\n");

        vprint("Unsupported filesystem type: ");
        vprint(fs_type_str);
        vprint("\n");
        mounted_fs = 0;
        return -1;
    }

    serial_puts("[FS] Filesystem mounted successfully\n");

    return 0;
}

int fs_detect_usb_drives(void) {
    return 0; // Placeholder
}

int fs_detect_sata_drives(void) {
    serial_puts("[FS] Detecting SATA drives\n");
    // SATA drives are handled by AHCI driver, detection happens during init
    // This function could be used for additional SATA-specific detection
    return 0;
}

void fs_set_boot_drive(unsigned int drive) {
    boot_drive = drive;
}

void fs_list_drives(void) {
    serial_puts("[FS] Listing available drives\n");
    vprint("Available drives:\n");
    // Probe drives 0-3 (ATA, including boot drive) and 4+ (SATA)
    // In a real implementation, this would detect actual connected drives
    for (int drive = 0; drive < (ATA_DRIVE_COUNT + 1 + 4); drive++) { // ATA drives + SATA drives
        serial_puts("[FS] Probing drive ");
        char drive_str[2] = {'0' + drive, '\0'};
        serial_puts(drive_str);
        serial_puts(": ");
        vprint("Probing drive ");
        vprint(drive_str);
        vprint(": ");
        unsigned char buffer[SECTOR_SIZE];
        int result = (drive >= SATA_DRIVE_OFFSET) ?
            ahci_read_sector(drive - SATA_DRIVE_OFFSET, 0, buffer) :
            ata_read_sector_drive(drive, 0, buffer);
        if (result == 0) {
            serial_puts("read OK, signature=");
            char sig_str[6];
            uint16_t boot_sig = (buffer[511] << 8) | buffer[510];
            int_to_str(boot_sig, sig_str);  // Boot signature at offset 510-511
            serial_puts(sig_str);
            serial_puts("\n");
            vprint("read OK, signature=");
            vprint(sig_str);
            vprint("\n");
            // Check if it's a valid boot sector (0xAA55 at offset 510-511)
            if (boot_sig == 0xAA55) {  // Valid boot sector signature
                FATBootSector* bs = (FATBootSector*)buffer;  // Define bs here
                // Determine drive type based on drive number
                if (drive >= SATA_DRIVE_OFFSET) {
                    vprint("sata");
                } else if (drive == 0) {  // Boot drive is always drive 0
                    vprint("hd");
                } else {
                    vprint("hd");
                }
                char drive_num[2] = {'1' + drive, '\0'};
                vprint(drive_num);
                vprint(" - ");

                // Print filesystem type based on boot sector structure
                if (bs->root_entries == 0 && bs->sectors_per_fat_32 > 0) {
                    vprint("FAT32");
                    serial_puts("[FS] Detected FAT32\n");
                } else if (bs->root_entries > 0 && bs->sectors_per_fat_16 > 0) {
                    vprint("FAT16");
                    serial_puts("[FS] Detected FAT16\n");
                } else {
                    vprint("Unknown");
                    serial_puts("[FS] Unknown filesystem\n");
                }

                // Print size info
                unsigned int total_sectors = bs->total_sectors_32;
                if (total_sectors == 0) total_sectors = bs->total_sectors_16;
                unsigned long long total_bytes = (unsigned long long)total_sectors * SECTOR_SIZE;

                vprint(" - ");
                if (total_bytes >= 1024ULL * 1024 * 1024) {  // GB
                    unsigned int size_gb = total_bytes / (1024ULL * 1024 * 1024);
                    char size_str[16];
                    int_to_str(size_gb, size_str);
                    vprint(size_str);
                    vprint(" GB");
                } else if (total_bytes >= 1024 * 1024) {  // MB
                    unsigned int size_mb = total_bytes / (1024 * 1024);
                    char size_str[16];
                    int_to_str(size_mb, size_str);
                    vprint(size_str);
                    vprint(" MB");
                } else if (total_bytes >= 1024) {  // KB
                    unsigned int size_kb = total_bytes / 1024;
                    char size_str[16];
                    int_to_str(size_kb, size_str);
                    vprint(size_str);
                    vprint(" KB");
                } else {  // Bytes
                    char size_str[16];
                    int_to_str(total_bytes, size_str);
                    vprint(size_str);
                    vprint(" bytes");
                }

                vprint("\n");
            } else {
                vprint("Invalid sector data or critical error\n");
                serial_puts("[FS] Invalid sector data or critical error\n");
            }
        } else {
            vprint("un-exists or hardly damaged\n");
            serial_puts("read failed\n");
        }
    }
}

int fs_format(unsigned int drive, const char* fs_type) {
    serial_puts("[FS] Formatting drive ");
    char drive_str[2] = {'0' + drive, '\0'};
    serial_puts(drive_str);
    serial_puts(" as ");
    serial_puts(fs_type);
    serial_puts("\n");

    // For now, only support FAT32 formatting
    if (strcmp_own_hell(fs_type, "fat32") != 0) {
        vprint("Error: Only FAT32 formatting is supported\n");
        serial_puts("[FS] Error: Only FAT32 formatting is supported\n");
        return -1;
    }

    vprint("Formatting drive ");
    vprint(drive_str);
    vprint(" as FAT32...\n");

    // Get drive size by reading sector 0 (even if not formatted)
    unsigned char buffer[SECTOR_SIZE];
    int read_result = (drive >= SATA_DRIVE_OFFSET) ?
        ahci_read_sector(drive - SATA_DRIVE_OFFSET, 0, buffer) :
        ata_read_sector_drive(drive, 0, buffer);
    if (read_result != 0) {
        vprint("Error: Cannot read drive\n");
        return -1;
    }

    // Calculate drive size (assume we can get it from somewhere, for now use a fixed size)
    // In a real implementation, this would probe the drive
    unsigned int total_sectors = 20480;  // 10MB for testing, adjust as needed

    // Create FAT32 boot sector
    FATBootSector bs = {0};
    bs.jump[0] = 0xEB;
    bs.jump[1] = 0x58;
    bs.jump[2] = 0x90;
    memcpy(bs.oem_name, "XMD     ", 8);
    bs.bytes_per_sector = SECTOR_SIZE;
    bs.sectors_per_cluster = 1;  // Small cluster size for testing
    bs.reserved_sectors = 32;
    bs.num_fats = 2;
    bs.root_entries = 0;  // FAT32 has no root directory entries
    bs.total_sectors_16 = 0;
    bs.media_descriptor = 0xF8;
    bs.sectors_per_fat_16 = 0;
    bs.sectors_per_track = 63;
    bs.num_heads = 255;
    bs.hidden_sectors = 0;
    bs.total_sectors_32 = total_sectors;
    bs.sectors_per_fat_32 = (total_sectors - bs.reserved_sectors) / (bs.sectors_per_cluster * 2 + 512 / SECTOR_SIZE) + 1;  // Rough calculation
    bs.flags = 0;
    bs.version = 0;
    bs.root_cluster = 2;
    bs.fs_info_sector = 1;
    bs.backup_boot_sector = 6;
    memset(bs.reserved, 0, 12);
    bs.drive_number = drive;
    bs.reserved1 = 0;
    bs.boot_signature = 0x29;
    bs.volume_id = 0x12345678;
    memcpy(bs.volume_label, "NO NAME    ", 11);
    memcpy(bs.fs_type, "FAT32   ", 8);

    // Debug: print what we're about to write
    serial_puts("[FS] Writing boot sector with fs_type: ");
    for (int i = 0; i < 8; i++) {
        char hex[3];
        int_to_str((unsigned char)bs.fs_type[i], hex);
        if (hex[1] == '\0') {
            hex[1] = ' ';
            hex[2] = '\0';
        }
        serial_puts(hex);
        serial_puts(" ");
    }
    serial_puts("\n");

    // Write boot sector
    int write_result = (drive >= SATA_DRIVE_OFFSET) ?
        ahci_write_sector(drive - SATA_DRIVE_OFFSET, 0, (unsigned char*)&bs) :
        ata_write_sector_drive(drive, 0, (unsigned char*)&bs);
    if (write_result != 0) {
        vprint("Error: Cannot write boot sector\n");
        return -1;
    }

    // Debug: verify what was written
    unsigned char verify_buffer[SECTOR_SIZE];
    int verify_result = (drive >= SATA_DRIVE_OFFSET) ?
        ahci_read_sector(drive - SATA_DRIVE_OFFSET, 0, verify_buffer) :
        ata_read_sector_drive(drive, 0, verify_buffer);
    if (verify_result != 0) {
        serial_puts("[FS] Error: Cannot verify boot sector write\n");
    } else {
        FATBootSector* verify_bs = (FATBootSector*)verify_buffer;
        serial_puts("[FS] Verified boot sector fs_type: ");
        for (int i = 0; i < 8; i++) {
            char hex[3];
            int_to_str((unsigned char)verify_bs->fs_type[i], hex);
            if (hex[1] == '\0') {
                hex[1] = ' ';
                hex[2] = '\0';
            }
            serial_puts(hex);
            serial_puts(" ");
        }
        serial_puts("\n");
    }

    // Clear FAT tables (set all to 0 except first two entries)
    unsigned char fat_sector[SECTOR_SIZE] = {0};
    fat_sector[0] = bs.media_descriptor;
    fat_sector[1] = 0xFF;
    fat_sector[2] = 0xFF;
    fat_sector[3] = 0xFF;

    unsigned int fat_start = bs.reserved_sectors;
    for (unsigned int i = 0; i < bs.sectors_per_fat_32 * bs.num_fats; i++) {
        int fat_write_result = (drive >= SATA_DRIVE_OFFSET) ?
            ahci_write_sector(drive - SATA_DRIVE_OFFSET, fat_start + i, fat_sector) :
            ata_write_sector_drive(drive, fat_start + i, fat_sector);
        if (fat_write_result != 0) {
            vprint("Error: Cannot write FAT\n");
            return -1;
        }
        // Clear the sector for subsequent writes
        memset(fat_sector, 0, SECTOR_SIZE);
    }

    // Create root directory
    unsigned int root_start = bs.reserved_sectors + bs.sectors_per_fat_32 * bs.num_fats;
    unsigned char root_sector[SECTOR_SIZE] = {0};
    int root_write_result = (drive >= SATA_DRIVE_OFFSET) ?
        ahci_write_sector(drive - SATA_DRIVE_OFFSET, root_start, root_sector) :
        ata_write_sector_drive(drive, root_start, root_sector);
    if (root_write_result != 0) {
        vprint("Error: Cannot write root directory\n");
        return -1;
    }

    vprint("Drive formatted successfully\n");
    serial_puts("[FS] Drive formatted successfully\n");
    return 0;
}

int fs_list_root(void) {
    serial_puts("[FS] Listing root directory\n");
    if (!mounted_fs) {
        vprint("Error: No filesystem mounted\n");
        serial_puts("[FS] Error: No filesystem mounted\n");
        return -1;
    }

    unsigned char buffer[SECTOR_SIZE];

    if (mounted_fs->type == FS_FAT32) {
        unsigned int cluster = mounted_fs->fs.fat32.boot_sector.root_cluster;
        while (cluster != 0xFFFFFFFF) {  // Not end of chain
            unsigned int lba = cluster_to_lba_fat32(cluster);
            for (int s = 0; s < mounted_fs->fs.fat32.boot_sector.sectors_per_cluster; s++) {
                int dir_read_result = (mounted_fs->drive_number >= SATA_DRIVE_OFFSET) ?
                    ahci_read_sector(mounted_fs->drive_number - SATA_DRIVE_OFFSET, lba + s, buffer) :
                    ata_read_sector_drive(mounted_fs->drive_number, lba + s, buffer);
                if (dir_read_result != 0) {
                    vprint("Error: Failed to read directory sector\n");
                    return -1;
                }

                FATDirEntry* entries = (FATDirEntry*)buffer;
                for (int i = 0; i < SECTOR_SIZE / sizeof(FATDirEntry); i++) {
                    if (entries[i].name[0] == 0) break;  // End of directory
                    if (entries[i].name[0] == 0xE5) continue;  // Deleted
                    if (entries[i].attributes & 0x0F) continue;  // Special attributes

                    // Print name
                    char name[12];
                    for (int j = 0; j < 11; j++) name[j] = entries[i].name[j];
                    name[11] = '\0';
                    vprint(name);
                    if (entries[i].attributes & 0x10) {
                        vprint(" <DIR>");
                    } else {
                        vprint(" ");
                        // Print size
                        char size_str[16];
                        int_to_str(entries[i].file_size, size_str);
                        vprint(size_str);
                        vprint(" bytes");
                    }
                    vprint("\n");
                }
            }
            cluster = get_next_cluster_fat32(cluster);
        }
    } else if (mounted_fs->type == FS_FAT16) {
        // FAT16 root directory is fixed size, not cluster-based
        for (int s = 0; s < mounted_fs->fs.fat16.root_dir_sectors; s++) {
            int fat16_read_result = (mounted_fs->drive_number >= SATA_DRIVE_OFFSET) ?
                ahci_read_sector(mounted_fs->drive_number - SATA_DRIVE_OFFSET, mounted_fs->fs.fat16.root_dir_start + s, buffer) :
                ata_read_sector_drive(mounted_fs->drive_number, mounted_fs->fs.fat16.root_dir_start + s, buffer);
            if (fat16_read_result != 0) {
                vprint("Error: Failed to read directory sector\n");
                return -1;
            }

            FATDirEntry* entries = (FATDirEntry*)buffer;
            for (int i = 0; i < SECTOR_SIZE / sizeof(FATDirEntry); i++) {
                if (entries[i].name[0] == 0) break;  // End of directory
                if (entries[i].name[0] == 0xE5) continue;  // Deleted
                if (entries[i].attributes & 0x0F) continue;  // Special attributes

                // Print name
                char name[12];
                for (int j = 0; j < 11; j++) name[j] = entries[i].name[j];
                name[11] = '\0';
                vprint(name);
                if (entries[i].attributes & 0x10) {
                    vprint(" <DIR>");
                } else {
                    vprint(" ");
                    // Print size
                    char size_str[16];
                    int_to_str(entries[i].file_size, size_str);
                    vprint(size_str);
                    vprint(" bytes");
                }
                vprint("\n");
            }
        }
    } else {
        vprint("Error: Unsupported filesystem type for listing\n");
        return -1;
    }

    return 0;
}
