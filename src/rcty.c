// Recovery TTY
// Version:0.1
// Author:Lohrrrr

#include "rcty.h"
#include "drivers/video/video.h"
#include "drivers/keyboard/keyboard.h"
#include "api.h"
#include "drivers/bootparser/parser.h"
#include "drivers/fs/fs.h"

#define MAX_INPUT 256

void rcty_start(void) {
    vprint_color("WARNING: Emergency recovery shell\n", 0x4F);
    vprint_color("\n", 0x4F);
    vprint_color("Main system process failed to start, or this shell was invoked from the GRUB.\n", 0x4F);
    vprint_color("Use this shell only to repair or reinstall the system.\n", 0x4F);
    vprint_color("Some commands use low-level drivers and may\n", 0x4F);
    vprint_color("cause data loss or hardware damage. Use at your\n", 0x4F);
    vprint_color("own risk; developers are not liable.\n", 0x4F);
    vprint_color("\n", 0x4F);
    vprint_color("Type 'yes' to continue. Any other input will\n", 0x4F);
    vprint_color("halt the kernel for safety.\n", 0x4F);
    vprint("> ");
    char input[MAX_INPUT];
    int input_len = 0;
    int safety_passed = 0;

    while (1) {
        char c = keyboard_input();
        if (c == '\n') {
            input[input_len] = '\0';
            if (!safety_passed) {
                if (strcmp_own_hell(input, "yes") == 0) {
                    safety_passed = 1;
                    vprint("\nSafety check passed. Entering rcty...\n");
                    vprint("rcty> ");
                } else {
                    vprint("\nSystem halted. You can now safely power off your PC.\n");
                    halt_krnl();
                }
            } else {
                // Parse command
                if (strcmp_own_hell(input, "clear") == 0) {
                    vclear();
                } else if (strcmp_own_hell(input, "help") == 0) {
                    vprint("\nAvailable commands:\n");
                    vprint("  clear - Clear the screen\n");
                    vprint("  echo <text> - Echo the text\n");
                    vprint("  help - Show this help\n");
                    vprint("  grubbootinf - Show GRUB boot arguments\n");
                    vprint("  mount <0-7> - Mount a drive by number\n");
                    vprint("  format <0-7> fat32 - Format a drive as FAT32\n");
                    vprint("  ls - List root directory contents\n");
                    vprint("  drives - List all available drives\n");
                } else if (input_len > 5 && input[0] == 'e' && input[1] == 'c' && input[2] == 'h' && input[3] == 'o' && input[4] == ' ') {
                    vprint("\n");
                    vprint(input + 5);
                } else if (strcmp_own_hell(input, "grubbootinf") == 0) {
                    vprint("\n");
                    vprint(grub_args());
                } else if (input_len > 6 && input[0] == 'm' && input[1] == 'o' && input[2] == 'u' && input[3] == 'n' && input[4] == 't' && input[5] == ' ') {
                    // mount <drive_number>
                    int drive_num = input[6] - '0';
                    if (drive_num >= 0 && drive_num <= 7) {
                        vprint("\nMounting drive ");
                        char drive_str[2] = {'0' + drive_num, '\0'};
                        vprint(drive_str);
                        vprint("...\n");
                        if (fs_mount(drive_num) != 0) {
                            vprint("Failed to mount drive\n");
                        } else {
                            vprint("Drive mounted successfully\n");
                        }
                    } else {
                        vprint("\nInvalid drive number. Use mount <0-7>\n");
                    }
                } else if (input_len > 8 && input[0] == 'f' && input[1] == 'o' && input[2] == 'r' && input[3] == 'm' && input[4] == 'a' && input[5] == 't' && input[6] == ' ') {
                    // format <drive_number> <fs_type>
                    int drive_num = input[7] - '0';
                    if (drive_num >= 0 && drive_num <= 7 && input[8] == ' ') {
                        const char* fs_type = input + 9;
                        vprint("\nFormatting drive ");
                        char drive_str[2] = {'0' + drive_num, '\0'};
                        vprint(drive_str);
                        vprint(" as ");
                        vprint(fs_type);
                        vprint("...\n");
                        if (fs_format(drive_num, fs_type) != 0) {
                            vprint("Failed to format drive\n");
                        } else {
                            vprint("Drive formatted successfully\n");
                        }
                    } else {
                        vprint("\nInvalid format. Use format <0-7> <fat32>\n");
                    }
                } else if (strcmp_own_hell(input, "ls") == 0) {
                    vprint("\nListing root directory:\n");
                    if (fs_list_root() != 0) {
                        vprint("Failed to list root directory. Is USB drive mounted?\n");
                    }
                } else if (strcmp_own_hell(input, "drives") == 0) {
                    vprint("\n");
                    fs_list_drives();
                } else {
                    vprint("\nUnknown command: ");
                    vprint(input);
                }
                vprint("\nrcty> ");
            }
            input_len = 0;
        } else if (c == '\b' && input_len > 0) {
            input_len--;
            vprint("\b");
        } else if (c != 0 && c != '\b' && input_len < MAX_INPUT - 1) {
            input[input_len++] = c;
            char echo[2] = {c, '\0'};
            vprint(echo);
        }
    }
}
