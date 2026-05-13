#ifndef FS_FAT32_H
#define FS_FAT32_H

#include <stdint.h>
#include "drivers/display/kprintf.h"
#include "fs/vfs.h"
#include "fs/fat32/fat32_vfs.h"

// ==========================================
// FAT32 BIOS Parameter Block (BPB)
// ==========================================
struct FAT32_BPB {
    // Standard DOS 2.0 BPB
    uint8_t  jump_boot[3];
    uint8_t  oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_count;
    uint16_t root_entry_count;    // Always 0 for FAT32
    uint16_t total_sectors_16;    // Always 0 for FAT32
    uint8_t  media_descriptor;
    uint16_t sectors_per_fat_16;  // Always 0 for FAT32
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;

    // FAT32 Extended BPB
    uint32_t sectors_per_fat_32;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_cluster;        // Usually cluster 2
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;      // Should be 0x28 or 0x29
    uint32_t volume_id;
    uint8_t  volume_label[11];    // Padded with spaces
    uint8_t  fs_type[8];          // Usually "FAT32   "
} __attribute__((packed));

// ==========================================
// FAT32 Directory Entry
// ==========================================
struct FAT_DirectoryEntry {
    uint8_t  name[11];            // 8.3 Filename (8 chars name, 3 chars extension)
    uint8_t  attributes;          // 0x10 = Directory, 0x20 = Archive, 0x0F = Long File Name
    uint8_t  reserved;
    uint8_t  creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;    // Renamed to match standard FAT32 specification
    uint16_t cluster_high;        // High 16 bits of the starting cluster
    uint16_t last_write_time;     // Renamed to match standard FAT32 specification
    uint16_t last_write_date;     // Renamed to match standard FAT32 specification
    uint16_t cluster_low;         // Low 16 bits of the starting cluster
    uint32_t size;                // File size in bytes (0 for directories)
} __attribute__((packed));

// ==========================================
// FAT32 Driver API
// ==========================================

// Initializes the FAT32 driver starting from a specific partition LBA
void fat32_init(uint32_t partition_lba_start);

// Prints the contents of the root directory for testing
void fat32_print_root_dir();

// Reads a file's raw data into a buffer starting from its first cluster
void fat32_read_file(uint32_t start_cluster, uint32_t size, uint8_t* out_buffer);

// Converts a FAT cluster number to a physical disk LBA sector
uint32_t cluster_to_lba(uint32_t cluster);

// Finds the first available cluster in the FAT table
uint32_t fat32_find_free_cluster();

// Writes a specific 32-bit value to a cluster's entry in the FAT table
void fat32_write_fat_entry(uint32_t cluster, uint32_t value);

// Updates the size and starting cluster of an existing file in the Root Directory
bool fat32_update_directory_entry(const char* filename, uint32_t start_cluster, uint32_t size);

#endif