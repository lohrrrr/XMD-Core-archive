// BootParser API
// Version:0.1
// Author:Lohrrrr

#ifndef PARSER_H
#define PARSER_H

#define NULL ((void*)0)

#define MAX_BOOT_ARGS 10

typedef struct {
    char* key;
    char* value;
} BootArg;

typedef struct {
    BootArg args[MAX_BOOT_ARGS];
    int count;
} BootArgsDict;

char* grub_args();
BootArgsDict* parse_boot_args();
int get_boot_arg_int(const char* key);
char* get_boot_arg_str(const char* key);

#endif
