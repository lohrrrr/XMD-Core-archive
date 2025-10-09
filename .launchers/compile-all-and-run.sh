#!/bin/bash
set -e

# ---------------------
# Компиляция всего
# ---------------------
sh .launchers/compile-all.sh

# ---------------------
# Запуск QEMU
# ---------------------
# Загружаем только bootloader, он сам подхватит kernel.bin с диска
qemu-system-i386 XMD.iso
