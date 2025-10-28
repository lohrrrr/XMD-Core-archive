# XMD OS Makefile
# Compiles all in one
# Can compile for now kernel, iso

# Compiler and tools
CC = gcc
AS = nasm
LD = ld
ISO_TOOL = grub-mkrescue
QEMU = qemu-system-i386

# Flags
CFLAGS = -I XMD-SRC -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -O2 -m32
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -nostdlib -m elf_i386

# Directories
SRC_DIR = XMD-SRC
BUILD_DIR = tmp
ISO_DIR = iso
XMD_DIR = XMD

# Source files
C_SOURCES = $(wildcard $(SRC_DIR)/kernel/*.c) \
            $(wildcard $(SRC_DIR)/kernel/*/*.c) \
            $(wildcard $(SRC_DIR)/drivers/*/*.c) \
            $(wildcard $(SRC_DIR)/drivers/*/*/*.c)

ASM_SOURCES = $(wildcard $(SRC_DIR)/kernel/*.asm)

# Object files
C_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
ASM_OBJECTS = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.o, $(ASM_SOURCES))

# All objects
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# Targets
.PHONY: all clean kernel iso run

all: kernel iso

kernel: $(XMD_DIR)/sysroot32/xkrnl

iso: $(BUILD_DIR)/xmd.iso

run: $(BUILD_DIR)/xmd.iso
	$(QEMU) -cdrom $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/kernel.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@

$(XMD_DIR)/sysroot32/xkrnl: $(BUILD_DIR)/kernel.elf
	cp $< $@

$(BUILD_DIR)/xmd.iso: $(XMD_DIR)/sysroot32/xkrnl
	@mkdir -p $(ISO_DIR)
	cp -r $(XMD_DIR)/* $(ISO_DIR)/
	$(ISO_TOOL) -o $@ $(ISO_DIR)
	rm -rf $(ISO_DIR)

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)
	rm -rf tmp
