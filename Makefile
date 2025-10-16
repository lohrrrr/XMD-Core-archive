# Makefile for building XMD project

# Variables
SRC_DIR = src
OBJ_DIR = obj
BUILD_DIR = compiled
BOOT_DIR = $(BUILD_DIR)/boot
ISO_NAME = ./XMD.iso
CC = gcc
LD = ld
ASM = nasm
RM = rm -rf
QEMU = qemu-system-i386

# Object files
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(shell find $(SRC_DIR) -name '*.c')) \
            $(OBJ_DIR)/boot.o
            
# Targets
.PHONY: all clean run

all: $(ISO_NAME) # Default target to build the ISO

$(ISO_NAME): $(BOOT_DIR)/kernel
	@mkdir -p $(BOOT_DIR)
	grub-mkrescue -o $@ $(BUILD_DIR)

$(BOOT_DIR)/kernel: $(OBJ_DIR)/kernel.elf
	@mkdir -p $(BOOT_DIR)
	mv $< $@

$(OBJ_DIR)/kernel.elf: $(OBJ_FILES)
	$(LD) -m elf_i386 -T linker.ld -o $@ $^

$(OBJ_DIR)/boot.o: $(SRC_DIR)/boot.asm
	$(ASM) -f elf32 $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -m32 -fno-stack-protector -fno-builtin -ffreestanding -O2 -c $< -o $@

# Run QEMU with the created ISO
run: $(ISO_NAME)
	$(QEMU) -cdrom $(ISO_NAME) -boot d -m 512 -smp 2

# Clean up temporary files
clean:
	$(RM) $(OBJ_DIR) $(BOOT_DIR) $(BUILD_DIR) $(ISO_NAME)
