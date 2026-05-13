#ifndef FS_EXFAT_H
#define FS_EXFAT_H

#include <stdint.h>
#include "drivers/display/kprintf.h"
#include "fs/vfs.h"
#include "fs/exfat/exfat_vfs.h"

// ==========================================
// ExFAT Boot Sector
// ==========================================
// ExFAT replaces the classic BPB with a completely different layout.
// The OEM name field always contains "EXFAT   " (with 3 trailing spaces).
// All size/offset fields are in SECTORS, not bytes.
struct ExFAT_BootSector {
    uint8_t  jump_boot[3];
    uint8_t  oem_name[8];              // Always "EXFAT   "
    uint8_t  must_be_zero[53];
    uint64_t partition_offset;         // LBA of this partition on the physical disk
    uint64_t volume_length;            // Total volume size in sectors
    uint32_t fat_offset;               // FAT start in sectors (from partition start)
    uint32_t fat_length;               // FAT size in sectors
    uint32_t cluster_heap_offset;      // Data region start in sectors (from partition start)
    uint32_t cluster_count;            // Total usable clusters
    uint32_t root_cluster;             // First cluster of the root directory
    uint32_t volume_serial;
    uint16_t fs_revision;              // Should be 0x0100
    uint16_t volume_flags;
    uint8_t  bytes_per_sector_shift;   // log2(bytes_per_sector). 9 = 512B, 12 = 4096B
    uint8_t  sectors_per_cluster_shift;// log2(sectors_per_cluster)
    uint8_t  num_fats;                 // 1 (or 2 for TexFAT)
    uint8_t  drive_select;
    uint8_t  percent_in_use;
    uint8_t  reserved[7];
} __attribute__((packed));

// ==========================================
// ExFAT Directory Entry Types
// ==========================================
// Unlike FAT32's fixed 32-byte records, ExFAT uses "entry sets":
//   - One PRIMARY entry  (type 0x85 = File)
//   - One SECONDARY entry (type 0xC0 = Stream Extension)
//   - One or more SECONDARY entries (type 0xC1 = File Name, 15 UTF-16 chars each)
// All entries are still 32 bytes wide.

#define EXFAT_ENTRY_FILE        0x85
#define EXFAT_ENTRY_STREAM      0xC0
#define EXFAT_ENTRY_NAME        0xC1
#define EXFAT_ENTRY_BITMAP      0x81
#define EXFAT_ENTRY_UPCASE      0x82
#define EXFAT_ENTRY_LABEL       0x83
#define EXFAT_ENTRY_EOD         0x00   // End Of Directory
#define EXFAT_ENTRY_DELETED     0x05   // Deleted file entry (bit 7 cleared on 0x85)

#define EXFAT_ATTR_DIRECTORY    0x10
#define EXFAT_ATTR_ARCHIVE      0x20
#define EXFAT_ATTR_READ_ONLY    0x01

// Generic 32-byte entry shell (used to peek at entry_type before casting)
struct ExFAT_GenericEntry {
    uint8_t entry_type;
    uint8_t data[31];
} __attribute__((packed));

// Primary File Entry (type 0x85)
struct ExFAT_FileEntry {
    uint8_t  entry_type;           // 0x85
    uint8_t  secondary_count;      // How many secondary entries follow (min 2)
    uint16_t set_checksum;         // Checksum of the whole entry set
    uint16_t file_attributes;      // EXFAT_ATTR_*
    uint16_t reserved1;
    uint32_t create_time;          // DOS-style packed timestamp
    uint32_t modified_time;
    uint32_t access_time;
    uint8_t  create_time_10ms;
    uint8_t  modified_time_10ms;
    uint8_t  create_utc_offset;
    uint8_t  modified_utc_offset;
    uint8_t  access_utc_offset;
    uint8_t  reserved2[7];
} __attribute__((packed));

// Stream Extension Entry (type 0xC0) — holds data location and lengths
struct ExFAT_StreamEntry {
    uint8_t  entry_type;           // 0xC0
    uint8_t  general_flags;        // Bit 1: NoFatChain (contiguous allocation)
    uint8_t  reserved1;
    uint8_t  name_length;          // Filename length in UTF-16 characters
    uint16_t name_hash;            // Hash of the uppercased filename
    uint16_t reserved2;
    uint64_t valid_data_length;    // Bytes actually written (≤ data_length)
    uint32_t reserved3;
    uint32_t first_cluster;        // Starting cluster of file data
    uint64_t data_length;          // Allocated size in bytes
} __attribute__((packed));

// File Name Entry (type 0xC1) — up to 15 UTF-16LE characters per entry
struct ExFAT_NameEntry {
    uint8_t  entry_type;           // 0xC1
    uint8_t  general_flags;
    uint16_t name[15];             // UTF-16LE characters
} __attribute__((packed));

// ==========================================
// ExFAT Driver API
// ==========================================

// Initializes the ExFAT driver starting from a specific partition LBA
void exfat_init(uint32_t partition_lba_start);

// Prints the contents of the root directory for debugging
void exfat_print_root_dir();

// Reads file data into a buffer, following the FAT chain if needed
void exfat_read_file(uint32_t start_cluster, uint64_t size, uint8_t* out_buffer);

// Converts a cluster number to a physical LBA sector
uint32_t exfat_cluster_to_lba(uint32_t cluster);

// Finds the first free cluster in the Allocation Bitmap / FAT
uint32_t exfat_find_free_cluster();

// Writes a 32-bit value to a cluster's FAT entry
void exfat_write_fat_entry(uint32_t cluster, uint32_t value);

// Updates an existing directory entry's cluster and size on disk
bool exfat_update_directory_entry(const char* filename, uint32_t start_cluster, uint64_t size);

#endif