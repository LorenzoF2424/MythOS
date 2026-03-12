#!/bin/bash
set -e

# Variables
OSDev=$(realpath "$(dirname "$0")/../..")
Myth="$OSDev/MythOS"
export PATH="$OSDev/cross-compiler/cross-gcc/bin:$PATH"
SOURCE="$Myth/src"
TEMP="$Myth/tempfiles"
OSFILENAME="MythicOS" 
bbP="$Myth/bootbootstuff"


ASSEMBLE="nasm -f elf64"
CXX="x86_64-elf-g++"
LD="x86_64-elf-ld"
CXXFLAGS="-std=c++17 -I include -ffreestanding -mcmodel=large -mno-red-zone -fstack-protector-all -fno-exceptions -fno-rtti -c"
LDFLAGS="-m elf_x86_64 -T linker.ld -nostdlib -z max-page-size=0x1000 -static -o"

COMPILE="$CXX $CXXFLAGS"
LINK="$LD $LDFLAGS"

#code

echo "==================================================================="
echo "Building $OSFILENAME..."
echo "==================================================================="
echo ""

echo "OSDev = $OSDev"
echo "Myth = $Myth"
echo ""

echo "1) Assembling Interrupts..."
OBJ_FILES=""
for asm_file in $(find "$SOURCE" -name "*.asm"); do
    
    filename=$(basename "$asm_file" .asm)
    
    obj_file="$TEMP/${filename}.o"
    
    echo "   -> $filename.asm"
    $ASSEMBLE "$asm_file" -o "$obj_file"
    
    OBJ_FILES="$OBJ_FILES $obj_file"
done        
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
