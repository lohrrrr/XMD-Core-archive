
// ASM: calls defined here
// ver: 0.1
// author: lohrrrr

void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void halt() {
    while (1) {
        __asm__ volatile ("hlt");
    }
}

