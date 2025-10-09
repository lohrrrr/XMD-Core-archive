#include "shell.h"
#include "commands.h"
#include "../kernel.h"

#define MAX_CMD_LEN 64
#define KBD_PORT 0x60

int shell_loop = 1;

static unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static char scancode_to_ascii(unsigned char sc) {
    switch (sc) {
        case 0x02: return '1';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';
        case 0x09: return '8';
        case 0x0A: return '9';
        case 0x0B: return '0';
        case 0x10: return 'q';
        case 0x11: return 'w';
        case 0x12: return 'e';
        case 0x13: return 'r';
        case 0x14: return 't';
        case 0x15: return 'y';
        case 0x16: return 'u';
        case 0x17: return 'i';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x1E: return 'a';
        case 0x1F: return 's';
        case 0x20: return 'd';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';
        case 0x2C: return 'z';
        case 0x2D: return 'x';
        case 0x2E: return 'c';
        case 0x2F: return 'v';
        case 0x30: return 'b';
        case 0x31: return 'n';
        case 0x32: return 'm';
        case 0x39: return ' ';
        case 0x1C: return '\n';
        default: return 0;
    }
}

void read_line(char* buffer, int max_len) {
    int len = 0;
    kprint("XMD> ");

    while (1) {
        unsigned char sc = inb(KBD_PORT);

        // Пропускаем break-коды (отпускание клавиш)
        if (sc & 0x80) continue;

        // Обработка Backspace
        if (sc == 0x0E) {
            if (len > 0) {
                len--;
                buffer[len] = 0;

                // Стираем символ на экране (возврат, пробел, возврат)
                kprint("\b \b");
            }
            // Ждем отпускания Backspace, чтобы не спамил
            while ((inb(KBD_PORT) & 0x80) == 0);
            continue;
        }

        char c = scancode_to_ascii(sc);
        if (!c) continue;

        if (c == '\n') {
            buffer[len] = 0;
            kprint("\n");

            // Ждём отпускания Enter, чтобы не сработало повторно
            while ((inb(KBD_PORT) & 0x80) == 0);
            return;
        }

        if (len < max_len - 1) {
            buffer[len++] = c;
            char s[2] = { c, 0 };
            kprint(s);
        }

        // Ждём отпускания клавиши (break-код) — предотвращает спам
        while ((inb(KBD_PORT) & 0x80) == 0);
    }
}

void InitShell() {
    char line[MAX_CMD_LEN];
    while (shell_loop) {
        read_line(line, MAX_CMD_LEN);

        // Разделяем команду и аргументы
        char* args = line;
        char* space = line;
        while (*space && *space != ' ') space++;
        if (*space) {
            *space = 0;
            args = space + 1;
        } else {
            args = "";
        }

        RunCommand(line, args);
    }
}
