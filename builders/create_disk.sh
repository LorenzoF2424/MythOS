#!/bin/bash

# 1. Create an empty 32 MB disk filled with zeros
echo "Creating raw disk..."
dd if=/dev/zero of=hdd.img bs=1M count=32 status=none

# 2. Create an MBR partition table with 1 FAT32 partition (type 'c')
echo "Writing MBR..."
sfdisk hdd.img <<< "type=c" > /dev/null 2>&1

# 3. Attach the img file as a loop device (like plugging in a physical USB drive)
echo "Attaching disk to host system..."
LOOP_DEV=$(sudo losetup -Pf --show hdd.img)

# 4. Format the partition ('p1') to FAT32
echo "Formatting to FAT32..."
sudo mkfs.fat -F 32 ${LOOP_DEV}p1 > /dev/null

# 5. Mount the partition and add some test files!
echo "Inserting test files..."
sudo mkdir -p /mnt/mythos_test
sudo mount ${LOOP_DEV}p1 /mnt/mythos_test

# Create a text file and a directory for your kernel to read
sudo bash -c 'echo "This is a secret file!" > /mnt/mythos_test/SECRET.TXT'
sudo mkdir /mnt/mythos_test/SYSTEM

# 6. Safely unmount everything
sudo umount /mnt/mythos_test
sudo losetup -d $LOOP_DEV

echo "Done! The disk 'hdd.img' is ready for MythOS."