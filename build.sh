#!/bin/bash
set -e

# Variables
SOURCE="./src/kernel"
TEMP="./tempfiles"
export PATH="/home/lorenzo/OSDev/cross-compiler/cross-gcc/bin:$PATH"
OSFILENAME="MythicOS" 
bbP="./bootbootstuff"


ASSEMBLE="nasm -f elf64"
CXX="x86_64-elf-g++"
LD="x86_64-elf-ld"
CXXFLAGS="-std=c++17 -I src/kernel -ffreestanding -mcmodel=large -mno-red-zone -fstack-protector-all -fno-exceptions -fno-rtti -c"
LDFLAGS="-m elf_x86_64 -T linker.ld -nostdlib -z max-page-size=0x1000 -static -o"

COMPILE="$CXX $CXXFLAGS"
LINK="$LD $LDFLAGS"

echo "==================================================================="
echo "Building $OSFILENAME with BOOTBOOT..."
echo "==================================================================="
echo ""

echo "1) Assembling Interrupts..."
$ASSEMBLE "$SOURCE/idt/interrupts.asm" -o "$TEMP/interrupts.o"
echo "-------------------------------------------------------------------"

echo "2) Compiling C++ kernel..."
OBJ_FILES=""

for cpp_file in $(find "$SOURCE" -name "*.cpp"); do
    
    filename=$(basename "$cpp_file" .cpp)
    
    obj_file="$TEMP/${filename}.o"
    
    echo "   -> $filename.cpp"
    $COMPILE "$cpp_file" -o "$obj_file"
    
    OBJ_FILES="$OBJ_FILES $obj_file"
done        
echo "-------------------------------------------------------------------"

echo "3) Linking kernel (ELF64 format)..."
$LINK \
        "$TEMP/kernel64.bin" \
        "$TEMP"/*.o
echo "-------------------------------------------------------------------"
