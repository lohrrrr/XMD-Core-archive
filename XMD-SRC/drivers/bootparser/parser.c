// BootParser
// Version:0.1
// Author:Lohrrrr

// This file parses boot arguments from GRUB.

#include "parser.h"

// Simple string functions since we can't use stdlib in kernel
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int atoi(const char* str) {
    int res = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        res = res * 10 + str[i] - '0';
    }
    return res;
}

char* strchr(const char* str, int c) {
    while (*str != (char)c) {
        if (*str == '\0') return NULL;
        str++;
    }
    return (char*)str;
}

char* grub_cmdline_global;

char* grub_args() { return grub_cmdline_global; }

BootArgsDict* parse_boot_args() {
    static BootArgsDict dict;
    static int parsed = 0;
    if (parsed) return &dict;
    dict.count = 0;

    if (!grub_cmdline_global) {
        parsed = 1;
        return &dict;
    }

    char* p = grub_cmdline_global;

    while (*p && dict.count < MAX_BOOT_ARGS) {
        // Skip leading spaces
        while (*p == ' ') p++;
        if (*p == '\0') break;

        char* start = p;
        // Find end of token
        while (*p && *p != ' ') p++;

        *p = '\0'; // Null-terminate the token permanently

        char* eq = NULL;
        char* colon = strchr(start, ':');
        char* equals = strchr(start, '=');
        if (colon && (!equals || colon < equals)) {
            eq = colon;
        } else if (equals) {
            eq = equals;
        }
        if (eq) {
            *eq = '\0';
            dict.args[dict.count].key = start;
            dict.args[dict.count].value = eq + 1;
            dict.count++;
        }

        p++; // Move to next character
    }

    parsed = 1;
    return &dict;
}

int get_boot_arg_int(const char* key) {
    BootArgsDict* dict = parse_boot_args();
    for (int i = 0; i < dict->count; i++) {
        if (strcmp(dict->args[i].key, key) == 0) {
            return atoi(dict->args[i].value);
        }
    }
    return 0; // Default value
}

char* get_boot_arg_str(const char* key) {
    BootArgsDict* dict = parse_boot_args();
    for (int i = 0; i < dict->count; i++) {
        if (strcmp(dict->args[i].key, key) == 0) {
            return dict->args[i].value;
        }
    }
    return NULL; // Not f0ound
}
