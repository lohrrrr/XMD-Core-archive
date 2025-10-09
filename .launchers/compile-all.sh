#!/bin/bash
# Shell-скрипт для сборки XMD с новой структурой проекта
# Исходники: /src
# Итоговые файлы: /compiled/boot (и ISO там же)

set -e  # Прерывание при любой ошибке

SRC_DIR="src"
OBJ_DIR="obj"
BUILD_DIR="compiled"
BOOT_DIR="$BUILD_DIR/boot"
ISO_NAME="./XMD.iso"

mkdir -p "$OBJ_DIR"
mkdir -p "$BOOT_DIR"

# Функция компиляции C-файлов
compile_c() {
    local src_file="$1"
    local out_file="$2"
    gcc -m32 -fno-stack-protector -fno-builtin -ffreestanding -O2 -c "$SRC_DIR/$src_file" -o "$OBJ_DIR/$out_file"
}

echo "Compiling kernel and modules..."
compile_c "kernel.c" "kernel.o"
compile_c "video/vga.c" "vga.o"
compile_c "video/gop.c" "gop.o"
compile_c "video.c" "video.o"
compile_c "debug.c" "debug.o"
compile_c "base_utils/shell.c" "shell.o"
compile_c "base_utils/commands.c" "commands.o"

echo "Assembling bootloader..."
nasm -f elf32 "$SRC_DIR/boot.asm" -o "$OBJ_DIR/boot.o"

echo "Linking ELF..."
ld -m elf_i386 -T linker.ld -o "$OBJ_DIR/kernel.elf" \
    "$OBJ_DIR/boot.o" \
    "$OBJ_DIR/kernel.o" \
    "$OBJ_DIR/vga.o" \
    "$OBJ_DIR/debug.o" \
    "$OBJ_DIR/shell.o" \
    "$OBJ_DIR/commands.o" \
    "$OBJ_DIR/gop.o" \
    "$OBJ_DIR/video.o"

# Перемещаем ELF в папку boot
mv "$OBJ_DIR/kernel.elf" "$BOOT_DIR/kernel"

echo "Creating ISO..."
grub-mkrescue -o "$ISO_NAME" "$BUILD_DIR"

# Очистка временных файлов
rm -rf "$OBJ_DIR"

echo "Build complete! ISO: $ISO_NAME"
