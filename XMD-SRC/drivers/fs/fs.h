// Filesystem driver API
// ver 0.1
// author: lohrrrr

#ifndef FS_H
#define FS_H

int fs_mount(unsigned int drive_num);
int fs_format(int drive_num, const char* fs_type);
int fs_list_root(void);
void fs_list_drives(void);

#endif

