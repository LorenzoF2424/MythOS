#!/bin/bash
set -e


# Variables
TEMP="./tempfiles"
OSFILENAME="MythOS"
RAM="256M"

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

echo "5) Cleaning up temporary files..."
rm -rf "$TEMP"/*
echo "-------------------------------------------------------------------"

echo "==================================================================="
echo "Starting $OSFILENAME on QEMU..."
echo "==================================================================="

qemu-system-x86_64 -drive format=raw,file=$OSFILENAME.img -m $RAM 2>/dev/null 
