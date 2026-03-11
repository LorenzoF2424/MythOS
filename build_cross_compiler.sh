#!/bin/bash
# Halt on errors
set -e

echo "==================================================================="
echo " Starting Cross-Compiler setup for MythicOS (x86_64-elf) "
echo "==================================================================="

# --- 1. Install Dependencies ---
echo "[1/5] Installing system dependencies (requires sudo)..."
sudo apt-get update -y
sudo apt-get install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo wget

# --- Configuration ---
export PREFIX="$HOME/OSDev/cross-compiler/cross-gcc"
export TARGET="x86_64-elf"
export PATH="$PREFIX/bin:$PATH"

# Versions
BINUTILS_VERSION="2.42"
GCC_VERSION="13.2.0"

# Working directory
WORKDIR="$HOME/OSDev/cross-compiler/src"
mkdir -p "$WORKDIR"
mkdir -p "$PREFIX"

cd "$WORKDIR"

# --- 2. Download sources ---
echo "[2/5] Downloading GCC and Binutils sources..."
if [ ! -f "binutils-$BINUTILS_VERSION.tar.gz" ]; then
    wget "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz"
fi

if [ ! -f "gcc-$GCC_VERSION.tar.gz" ]; then
    wget "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz"
fi

# --- 3. Extract archives ---
echo "[3/5] Extracting archives..."
if [ ! -d "binutils-$BINUTILS_VERSION" ]; then
    tar -xf binutils-$BINUTILS_VERSION.tar.gz
fi
if [ ! -d "gcc-$GCC_VERSION" ]; then
    tar -xf gcc-$GCC_VERSION.tar.gz
fi

CORES=$(nproc)

# --- 4. Build Binutils ---
echo "[4/5] Compiling Binutils..."
mkdir -p build-binutils
cd build-binutils
../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$CORES
make install
cd ..

# --- 5. Build GCC ---
echo "[5/5] Compiling GCC (this will take a while ☕)..."
mkdir -p build-gcc
cd build-gcc
../gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx

make all-gcc -j$CORES
make all-target-libgcc -j$CORES
make all-target-libstdc++-v3 -j$CORES

make install-gcc
make install-target-libgcc
make install-target-libstdc++-v3
cd ..

echo "==================================================================="
echo " DONE! The x86_64-elf cross-compiler is ready in:"
echo " $PREFIX/bin"
echo "==================================================================="