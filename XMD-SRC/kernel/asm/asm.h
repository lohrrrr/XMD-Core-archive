// ASM: calls defined here
// ver: 0.1
// author: lohrrrr

#ifndef ASM_H
#define ASM_H

void outb(unsigned short port, unsigned char val);
unsigned char inb(unsigned short port);
void halt(void);

#endif
