// Filesystem driver
// ver 0.1
// author: lohrrrr

#include "fs.h"

int fs_mount(unsigned int drive_num) {
    (void)drive_num; // Suppress unused parameter warning
    // Placeholder implementation
    return 0; // Success
}

int fs_format(int drive_num, const char* fs_type) {
    (void)drive_num; // Suppress unused parameter warning
    (void)fs_type; // Suppress unused parameter warning
    // Placeholder implementation
    return 0; // Success
}

int fs_list_root(void) {
    // Placeholder implementation
    return 0; // Success
}

void fs_list_drives(void) {
    // Placeholder implementation
}

