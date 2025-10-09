#include "commands.h"
#include "../kernel.h"
#include "../debug.h"
#include "shell.h"

struct Command commands[] = {
    {"help", cmd_help},
    {"info", cmd_info},
    {"clear", cmd_clear},
    {"reboot", cmd_reboot},
    {"halt", cmd_halt},
    {"rsf", cmd_rsof},
    {"exit", cmd_break},
    {0, 0}
};

void RunCommand(const char* name, const char* args) {
    for (int i = 0; commands[i].name; i++) {
        if (strcmp_own_hell(commands[i].name, name) == 0) {
            commands[i].func(args);
            return;
        }
    }
    kprint("Unknown command: "); kprint(name); kprint("\n");
}

void cmd_break(const char* args) {
    DPRINT("Exiting shell...");
    //shell_loop = 0;
}

void cmd_help(const char* args) {
    kprint("Available commands:\n");
    for (int i = 0; commands[i].name; i++) {
        kprint("  "); kprint(commands[i].name); kprint("\n");
    }
}

void cmd_info(const char* args) {
    kprint("XMD Kernel Base System\n");
}

void cmd_clear(const char* args) {
    kclear();
}

void cmd_reboot(const char* args) {
    unsigned char good = 0x02;
    while (good & 0x02) good = *(volatile unsigned char*)0x64;
    *(volatile unsigned char*)0x64 = 0xFE;
    for (;;) __asm__ __volatile__("hlt");
}

void cmd_halt(const char* args) {
    kprint("System halted.\n");
    for (;;) __asm__ __volatile__("hlt");
}

void cmd_rsof(const char* args) {
    Debug_RSOF("Manual RSOF command executed");
}
