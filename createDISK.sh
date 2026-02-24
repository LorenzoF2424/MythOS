#!/bin/bash
set -e

# Variabili
SOURCE="./SystemSource"
TEMP="./tempfiles"
export PATH="/home/lorenzo/OSDev/cross-compiler/cross-gcc/bin:$PATH"
OSFILENAME="MythicOS"

echo "==================================================================="
echo "Building $OSFILENAME Hard Disk with BOOTBOOT..."
echo "==================================================================="
echo ""

echo "1) Assembling Interrupts..."
nasm -f elf64 $SOURCE/syskernel/interrupts.asm -o $TEMP/interrupts.o
echo "-------------------------------------------------------------------"

echo "2) Compiling C++ kernel..."
x86_64-elf-g++ -ffreestanding -g -c "$SOURCE/syskernel/kernel64.cpp" -o "$TEMP/kernel64.o" \
   -O2 -Wall -Wextra -fno-exceptions -fno-rtti -mcmodel=large -mno-red-zone
echo "-------------------------------------------------------------------"

echo "3) Linking kernel (ELF64 format)..."
x86_64-elf-ld -m elf_x86_64 -T linker.ld -o "$TEMP/kernel64.bin" \
   "$TEMP/kernel64.o" "$TEMP/interrupts.o"
echo "-------------------------------------------------------------------"

echo "4) Generating bootable image with mkbootimg..."
./mkbootimg mkbootimg.json
echo "CREATION OF HARD DISK IMAGE \"$OSFILENAME.img\" SUCCESSFUL!!!!"
echo "-------------------------------------------------------------------"

echo "==================================================================="
echo "Starting $OSFILENAME.img on QEMU..."
echo "==================================================================="

qemu-system-x86_64 -drive format=raw,file=$OSFILENAME.img -m 128M -vga std