#!/bin/bash
set -e

# Variabili
SOURCE="./SystemSource"
TEMP="./tempfiles"
export PATH="/home/lorenzo/OSDev/cross-compiler/cross-gcc/bin:$PATH"
OSFILENAME="MythicOS" 
bbP="./bootbootstuff"

echo "==================================================================="
echo "Building $OSFILENAME with BOOTBOOT..."
echo "==================================================================="
echo ""

echo "1) Assembling Interrupts..."
nasm -f elf64 $SOURCE/syskernel/interrupts.asm -o $TEMP/interrupts.o
echo "-------------------------------------------------------------------"

echo "2) Compiling C++ kernel..."
x86_64-elf-g++ -std=c++17 -ffreestanding -mcmodel=large -mno-red-zone -fstack-protector-all -fno-exceptions -fno-rtti -c \
                    "$SOURCE/syskernel/kernel64.cpp" -o "$TEMP/kernel64.o"

x86_64-elf-g++ -std=c++17 -ffreestanding -mcmodel=large -mno-red-zone -fno-stack-protector -fno-exceptions -fno-rtti -c \
                   "$SOURCE/syskernel/ssp.cpp" -o "$TEMP/ssp.o"
echo "-------------------------------------------------------------------"



echo "3) Linking kernel (ELF64 format)..."
x86_64-elf-ld -m elf_x86_64 -T linker.ld -nostdlib -z max-page-size=0x1000 -static \
   -o "$TEMP/kernel64.bin" "$TEMP/kernel64.o" $TEMP/interrupts.o "$TEMP/ssp.o"



echo "-------------------------------------------------------------------"

echo "4) Generating bootable IMG with custom TAR initrd..."

rm -rf $TEMP/initrd
mkdir -p $TEMP/initrd/sys


cp $TEMP/kernel64.bin $TEMP/initrd/sys/core


cd $TEMP/initrd
tar -cvf ../initrd.tar sys
cd ../..

./bootbootstuff/mkboot ./bootbootstuff/mkbootDISK.json MythicOS.img

echo "CREATION SUCCESSFUL!!!!"
echo "-------------------------------------------------------------------"

echo "==================================================================="
echo "Starting $OSFILENAME on QEMU..."
echo "==================================================================="

qemu-system-x86_64 -drive format=raw,file=MythicOS.img -m 256M

