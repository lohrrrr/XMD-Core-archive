#ifndef COMMANDS_H
#define COMMANDS_H

struct Command {
    const char* name;
    void (*func)(const char*);
};

extern struct Command commands[];

void cmd_help(const char* args);
void cmd_info(const char* args);
void cmd_clear(const char* args);
void cmd_reboot(const char* args);
void cmd_halt(const char* args);
void cmd_rsof(const char* args);
void cmd_break(const char* args);

#endif
