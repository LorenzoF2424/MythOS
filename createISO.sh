#!/bin/bash
set -e


# Variables
TEMP="./tempfiles"
OSFILENAME="MythOS"
WORKDIR=$(pwd)
RAM="256M"

sed -i 's/\r$//' ./builders/build.sh
sudo ./builders/build.sh

echo "4) Preparazione initrd..."
rm -rf "$TEMP/initrd_build"
mkdir -p "$TEMP/initrd_build/sys"
cp "$TEMP/kernel64.bin" "$TEMP/initrd_build/sys/core"
echo "-------------------------------------------------------------------"

echo "5) Generating ISO with mkbootimg..."
cat > "$WORKDIR/mkbootISO_temp.json" << EOF
{
  "config": "$WORKDIR/bootbootstuff/bootbootISO.config",
  "initrd": { "type": "tar", "directory": "$WORKDIR/tempfiles/initrd_build" },
  "iso9660": true,
  "partitions": [
    { "type": "boot", "size": 16 }
  ]
}
EOF

"$WORKDIR/bootbootstuff/mkboot" "$WORKDIR/mkbootISO_temp.json" "$WORKDIR/$OSFILENAME.iso"

echo "CREATION SUCCESSFUL!!!!"
echo "-------------------------------------------------------------------"

echo "6) Cleaning up temporary files..."
rm -rf "$TEMP"/*
rm -f "$WORKDIR/mkbootISO_temp.json"
echo "-------------------------------------------------------------------"

echo "==================================================================="
echo "Starting $OSFILENAME on QEMU..."
echo "==================================================================="

qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom $OSFILENAME.iso -m $RAM 2>/dev/null