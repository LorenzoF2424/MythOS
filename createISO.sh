#!/bin/bash
set -e

# Variabili
SOURCE="./SystemSource"
TEMP="./tempfiles"
WORKDIR=$(pwd)
export PATH="/home/lorenzo/OSDev/cross-compiler/cross-gcc/bin:$PATH"
OSFILENAME="MythicOS"

echo "==================================================================="
echo "Building $OSFILENAME with BOOTBOOT..."
echo "==================================================================="
echo ""

echo "1) Assembling Interrupts..."
nasm -f elf64 $SOURCE/syskernel/interrupts.asm -o $TEMP/interrupts.o
echo "-------------------------------------------------------------------"

echo "2) Compiling C++ kernel..."
x86_64-elf-g++ -ffreestanding -mcmodel=large -mno-red-zone -fstack-protector-all -fno-exceptions -fno-rtti -c \
                    "$SOURCE/syskernel/kernel64.cpp" -o "$TEMP/kernel64.o"

x86_64-elf-g++ -ffreestanding -mcmodel=large -mno-red-zone -fno-stack-protector -fno-exceptions -fno-rtti -c \
                   "$SOURCE/syskernel/ssp.cpp" -o "$TEMP/ssp.o"
echo "-------------------------------------------------------------------"

echo "3) Linking kernel (ELF64 format)..."
x86_64-elf-ld -m elf_x86_64 -T linker.ld -nostdlib -z max-page-size=0x1000 -static \
   -o "$TEMP/kernel64.bin" "$TEMP/kernel64.o" $TEMP/interrupts.o "$TEMP/ssp.o"
echo "-------------------------------------------------------------------"

echo "4) Preparazione initrd..."

# Crea struttura initrd con kernel dentro sys/core
rm -rf $TEMP/initrd_build
mkdir -p $TEMP/initrd_build/sys
cp $TEMP/kernel64.bin $TEMP/initrd_build/sys/core

echo "-------------------------------------------------------------------"

echo "5) Generazione ISO con mkbootimg..."

cat > $WORKDIR/mkbootISO_temp.json << EOF
{
  "config": "$WORKDIR/bootbootstuff/bootbootISO.config",
  "initrd": { "type": "tar", "directory": "$WORKDIR/tempfiles/initrd_build" },
  "iso9660": true,
  "partitions": [
    { "type": "boot", "size": 16 }
  ]
}
EOF

$WORKDIR/bootbootstuff/mkboot $WORKDIR/mkbootISO_temp.json $WORKDIR/MythicOS.iso

echo "CREATION SUCCESSFUL!!!!"
echo "-------------------------------------------------------------------"

echo "==================================================================="
echo "Starting $OSFILENAME on QEMU..."
echo "==================================================================="

qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom MythicOS.iso -m 256M -vga std