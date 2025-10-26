# Kernel Built-ins

The XMD-Core kernel includes the following built-in software components:

## Core Kernel Functions
- **Main Entry Point**: `kmain()` - Initializes the kernel, sets up video, and starts the recovery console (RCTY).
- **Hardware I/O**: `outb()` - Low-level port output function for hardware communication.
- **String Comparison**: `strcmp_own_hell()` - Custom string comparison utility.
- **System Halt**: `halt_krnl()` - Safely halts the kernel execution.

## Recovery Console (RCTY)
- A basic command-line interface for system recovery and testing.
- Commands include:
  - `clear` - Clears the screen.
  - `help` - Displays available commands.
  - `echo <text>` - Echoes back the provided text.
  - `grubbootinf` - Shows GRUB boot arguments.
- Includes safety checks to prevent accidental use.

## Video Subsystem
- **Video Initialization**: Sets up VGA text mode by default.
- **Text Output**: Functions for printing text in color or default color, clearing screen.
- **VGA Driver**: Supports text mode (80x25), and placeholders for graphics modes (320x200x256 and 640x480x16(do not use, it can kill your videocard/monitor)).
- **Font Support**: Basic 8x8 font for graphics text rendering.

## Keyboard Input
- **Keyboard Driver**: Handles PS/2 keyboard input, converting scancodes to ASCII characters.
- Supports basic keys including letters, numbers, space, enter, and backspace.

## Boot Argument Parser
- Parses GRUB boot command-line arguments for kernel configuration.

## Filesystem Support
- **ATA/IDE Driver**: Low-level sector read/write operations for HDD access.
- **FAT16/FAT32 Support**: Mount and list root directory contents of FAT16 and FAT32 filesystems.
- **Generic Filesystem Interface**: Extensible design for adding support for other filesystem types (EXT2, NTFS planned).

## Bootloader Integration
- Multiboot-compliant assembly code for GRUB bootloader integration, setting up stack and calling `kmain()` with multiboot info pointer.

## ATA/IDE Driver
- Basic ATA/IDE driver for reading and writing sectors from hard drives.
- Supports primary master/slave drives, LBA addressing.
- Functions: `ata_init()`, `ata_read_sector()`, `ata_write_sector()`, `ata_read_sector_drive()`, `ata_write_sector_drive()`.

## FAT32 Filesystem Support
- FAT32 filesystem driver for mounting and reading FAT32 partitions.
- Supports mounting the boot device, listing root directory contents.
- Functions: `fs_mount()`, `fs_list_root()`.
- RCTY commands: `mount` to mount filesystem, `ls` to list root directory.

These components provide the essential low-level services for hardware abstraction, user interaction via console, and system initialization.
