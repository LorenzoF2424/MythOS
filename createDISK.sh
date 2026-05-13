#!/bin/bash
set -e

# Variables
TEMP="./tempfiles"
OSFILENAME="MythOS"
RAM="20M"
HDD_IMG="hdd.img"

sed -i 's/\r$//' ./builders/build.sh
sudo ./builders/build.sh

echo "4) Generating bootable IMG with custom TAR initrd..."

rm -rf $TEMP/initrd
mkdir -p $TEMP/initrd/sys

cp $TEMP/kernel64.bin $TEMP/initrd/sys/core

cd $TEMP/initrd
tar -cvf ../initrd.tar sys
cd ../..

./bootbootstuff/mkboot ./bootbootstuff/mkbootDISK.json $OSFILENAME.img

echo "CREATION SUCCESSFUL!!!!"
echo "-------------------------------------------------------------------"

# ==========================================
# FAT32 DATA DISK MANAGEMENT
# ==========================================
echo "4.5) Checking FAT32 Data Disk ($HDD_IMG)..."
if [ ! -f "$HDD_IMG" ]; then
    echo "Data disk not found. Starting creation..."
    ./builders/create_disk.sh
else
    echo "Data disk found and ready!"
fi
echo "-------------------------------------------------------------------"

echo "5) Cleaning up temporary files..."
rm -rf "$TEMP"/*
echo "-------------------------------------------------------------------"

echo "==================================================================="
echo "Starting $OSFILENAME on QEMU..."
echo "==================================================================="

# Start QEMU with TWO disks: 
# index=0 (Master) is the MythOS boot disk
# index=1 (Slave) is our FAT32 data disk
qemu-system-x86_64 \
    -drive format=raw,file=$OSFILENAME.img,index=0,media=disk \
    -drive format=raw,file=$HDD_IMG,index=1,media=disk \
    -m $RAM 2>/dev/null