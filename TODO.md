# XMD-Kernel

The XMD is uniqe sys-kernel because bare kernel already can run as OS. Simply because the core(kernel) is already an OS in some kind.
So in fact you maybe want to see API.c and API.h and do whatever u wamntasfopk,aslf


## Video modes:
- [ ] Implent VESA driver
- [ ] Implent GOP driver
- [Canceled] Implent VGA 12th and 13 modes to VGA driver
- [AfterVesa] Implent screen resolution change to the kernel

## Funcs:
- [InProgress] Add more stuff to kernel so uhhhh, the OS that uses it could use it...
- [Done] And uhhh make abstract lib so uhhhhh I can call "display_print()" anywhere and not manualy typing if (EFI) { VGA_Print(); } elseif (BIOS) { GOP_Print(); } else { RSOF("Idk from what are you booting, boot kernel with 'REDUCED_ERROR_HANDLING=1' to bypass it") }
- [Partly] Add API.c and API.h so the users can create a programms and os-ses for already compiled kernel.
