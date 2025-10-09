BITS 32

section .text
    ALIGN 4
    DD 0x1BADB002
    DD 0x00000000
    DD -(0x1BADB002 + 0x00000000)

global start
extern kmain
extern grub_cmdline_global

start:
    CLI
    MOV esp, stack_space
    MOV eax, [ebx+16]
    MOV [grub_cmdline_global], eax
    CALL kmain
    HLT
    
EbnutKernel:
    CLI
    HLT
    JMP EbnutKernel

section .bss
RESB 8192
stack_space: