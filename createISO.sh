#!/bin/bash

set -e
echo "Converting Folder to ISO File....."

SOURCE="./SystemSource"
DEST="./System"
TEMP="./tempfiles"
export PATH="/home/lorenzo/OSDev/cross-compiler/cross-gcc/bin:$PATH"
OSFILENAME="MythicOS"

# Crea directory temporanee
mkdir -p $TEMP
mkdir -p $DEST

echo "==================================================================="
echo "Building bootable ISO....."
echo "==================================================================="

echo "1) Assembling Bootloader....."
nasm -f bin $SOURCE/sysboot/bootloader.asm -o $TEMP/bootloader.bin || {
    echo "NASM bootloader compile ERROR!!!"
    exit 1
}
echo "Bootloader Assembly Successfull!!!"
echo "-------------------------------------------------------------------"

echo "2) Assembling Kernel Entry File....."
nasm -f elf32 $SOURCE/sysboot/kernel_entry.asm -o $TEMP/kernel_entry.o || {
    echo "NASM kernel_entry compile ERROR!"
    exit 1
}
echo "Kernel Entry Assembly Successfull!!!"
echo "-------------------------------------------------------------------"

echo "3) Compiling C++ kernel......"
x86_64-elf-g++ -ffreestanding -m32 -g -c "$SOURCE/syskernel/kernel32VGA.cpp" -o "$TEMP/kernel32VGA.o" \
   -O2 -Wall -Wextra -fno-exceptions -fno-rtti

x86_64-elf-ld -m elf_i386 -T linker.ld --oformat binary -o "$TEMP/kernel.bin" \
                    "$TEMP/kernel_entry.o" "$TEMP/kernel32VGA.o"
echo "-------------------------------------------------------------------"

echo "4) Creating combined boot image (bootloader + kernel)....."
# Unisci bootloader e kernel in un unico file
cat $TEMP/bootloader.bin $TEMP/kernel.bin > $TEMP/boot_combined.bin

# Padda a multipli di 512 byte
FILESIZE=$(stat -c%s "$TEMP/boot_combined.bin")
REMAINDER=$((FILESIZE % 512))
if [ $REMAINDER -ne 0 ]; then
    PADDING=$((512 - REMAINDER))
    dd if=/dev/zero bs=1 count=$PADDING >> $TEMP/boot_combined.bin
fi

# Calcola quanti settori da 512 byte servono
SECTORS=$(($(stat -c%s "$TEMP/boot_combined.bin") / 512))
echo "Combined file size: $(stat -c%s "$TEMP/boot_combined.bin") bytes ($SECTORS sectors)"
echo "-------------------------------------------------------------------"

echo "5) Creating bootable ISO....."
# Usa il file combinato come boot image
# -boot-load-size deve essere il numero di settori da 512 byte
mkisofs -R -J \
  -o ./$OSFILENAME.iso \
  -b boot_combined.bin \
  -c boot.cat \
  -no-emul-boot \
  -boot-load-size $SECTORS \
  -boot-info-table \
  $TEMP

echo "ISO Creation Successfull!"
echo "==================================================================="

echo "Starting QEMU....."
qemu-system-x86_64 -cdrom ./$OSFILENAME.iso