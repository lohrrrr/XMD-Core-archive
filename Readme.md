# XMD OS + XMD Kernel

## Introduction

XMD OS is a simple but working operating system that can be run by 32-bit and 64-bit processors. There's a plan to add 16-bit device support. There's also an ARMv7 branch, but it's currently broken.

## Features

XMD OS provides basic operating system functionality, including:

- **PLACEHOLDER**: PLACEHOLDER.


## Building

PLACEHOLDER

## Boot arguments

XMD supports serval boot agrguments listed below
- `serial=1`: Enable serial output.
- `rcty=1`: Enter recovery mode (see below).

## Recovery Mode (RCTY)

If the main system fails to start or is invoked via GRUB boot argument `rcty=1`, the kernel enters Recovery TTY (RCTY) mode. This is an emergency shell for repairing or reinstalling the system.

**Warning**: Use this shell only for repairs. Some commands may cause data loss or hardware damage. Developers are not liable.

To enter RCTY:
- Type 'yes' at the prompt to continue.
- Available commands:
  - `clear`: Clear the screen.
  - `echo <text>`: Echo the text.
  - `help`: Show help.
  - `grubbootinf`: Show GRUB boot arguments.
  - `mount <drive num>`: Mount a drive by number.
  - `format <drive num> fat32`: Format a drive as FAT32.
  - `ls`: List curent directory contents of mounted device.
  - `drives`: List all available drives.

