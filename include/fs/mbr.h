#ifndef FS_MBR_H
#define FS_MBR_H

#include <stdint.h>
#include "drivers/display/kprintf.h"
#include "fs/fat32/fat32.h"
#include "fs/exfat/exfat.h"

// Structure of a single partition entry (exactly 16 bytes)
struct PartitionEntry {
    uint8_t  status;         // 0x80 = Bootable (Active), 0x00 = Inactive
    uint8_t  chs_start[3];   // Cylinder-Head-Sector start (obsolete, ignored)
    uint8_t  type;           // File System type ID (e.g., 0x0B or 0x0C = FAT32)
    uint8_t  chs_end[3];     // Cylinder-Head-Sector end (obsolete)
    uint32_t lba_start;      // THE MOST IMPORTANT DATA: starting sector of the partition
    uint32_t sector_count;   // Partition size (total number of sectors)
} __attribute__((packed));

// Structure of the entire Master Boot Record (exactly 512 bytes)
struct MBR {
    uint8_t  boot_code[446];       // Bootloader machine code
    PartitionEntry partitions[4];  // The 4 primary partition slots
    uint16_t magic_signature;      // Must be 0xAA55
} __attribute__((packed));

// Function to scan the disk for partitions
void mbr_read_partitions();

#endif 