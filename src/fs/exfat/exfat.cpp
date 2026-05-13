#include "fs/exfat/exfat.h"
#include "drivers/storage/ata_pio.h"

// ==========================================
// ExFAT Internal State
// ==========================================
static uint32_t exfat_partition_lba;     // LBA where this partition starts
static uint32_t exfat_fat_lba;           // LBA of the FAT table
static uint32_t exfat_heap_lba;          // LBA of the cluster heap (data region)
static uint32_t exfat_root_cluster;      // First cluster of the root directory
static uint32_t exfat_bytes_per_sector;  // Usually 512 or 4096
static uint32_t exfat_sectors_per_cluster;

// ==========================================
// Driver Init
// ==========================================

void exfat_init(uint32_t partition_lba_start) {
    uint8_t buffer[512];

    // 1. Read the Volume Boot Record
    ata_pio_read_sector(partition_lba_start, buffer);
    ExFAT_BootSector* bs = (ExFAT_BootSector*)buffer;

    // Early return: verify OEM name
    // ExFAT always writes "EXFAT   " (5 chars + 3 spaces)
    const char expected[] = "EXFAT   ";
    for (int i = 0; i < 8; i++) {
        if (bs->oem_name[i] != (uint8_t)expected[i]) {
            kprintf("Error: Partition is not formatted as ExFAT.\n");
            return;
        }
    }

    // Early return: must_be_zero region must be all zeros
    for (int i = 0; i < 53; i++) {
        if (bs->must_be_zero[i] != 0) {
            kprintf("Error: ExFAT boot sector is corrupt (MustBeZero field violated).\n");
            return;
        }
    }

    // 2. Decode the shift fields — ExFAT stores log2 of sizes, not the sizes directly
    exfat_bytes_per_sector   = (uint32_t)1 << bs->bytes_per_sector_shift;
    exfat_sectors_per_cluster = (uint32_t)1 << bs->sectors_per_cluster_shift;

    // 3. Save the navigation landmarks
    exfat_partition_lba = partition_lba_start;
    exfat_fat_lba       = partition_lba_start + bs->fat_offset;
    exfat_heap_lba      = partition_lba_start + bs->cluster_heap_offset;
    exfat_root_cluster  = bs->root_cluster;

    // 4. Print volume info
    kprintf("\n=== ExFAT Volume Initialized ===\n");
    kprintf("Bytes per Sector    : %d\n", exfat_bytes_per_sector);
    kprintf("Sectors per Cluster : %d\n", exfat_sectors_per_cluster);
    kprintf("FAT Start LBA       : %d\n", exfat_fat_lba);
    kprintf("Heap Start LBA      : %d\n", exfat_heap_lba);
    kprintf("Root Cluster        : %d\n", exfat_root_cluster);
    kprintf("================================\n\n");

    // 5. Mount the root directory into the VFS
    ExFATDirectory* root_node = new ExFATDirectory("hdd", exfat_root_cluster);
    vfs_mount("hdd", root_node);
}

// ==========================================
// Cluster Navigation
// ==========================================

// ExFAT formula: heap_start + (cluster - 2) * sectors_per_cluster
// Clusters 0 and 1 are reserved, just like FAT32.
uint32_t exfat_cluster_to_lba(uint32_t cluster) {
    if (cluster < 2) return exfat_heap_lba;
    return exfat_heap_lba + ((cluster - 2) * exfat_sectors_per_cluster);
}

// Reads the next cluster number from the FAT chain.
// Returns 0xFFFFFFFF when we hit the End Of Chain marker.
static uint32_t exfat_next_cluster(uint32_t cluster) {
    if (cluster < 2) return 0xFFFFFFFF;

    uint8_t sector_buffer[512];

    // Each FAT entry is 4 bytes; 128 entries fit in a 512-byte sector
    uint32_t fat_sector_offset   = cluster / 128;
    uint32_t fat_index_in_sector = cluster % 128;

    ata_pio_read_sector(exfat_fat_lba + fat_sector_offset, sector_buffer);

    uint32_t* fat = (uint32_t*)sector_buffer;
    uint32_t  next = fat[fat_index_in_sector] & 0x0FFFFFFF;

    // 0x0FFFFFFF = End Of Chain in ExFAT
    return next;
}

// ==========================================
// File Reading
// ==========================================

void exfat_read_file(uint32_t start_cluster, uint64_t size, uint8_t* out_buffer) {
    // Early return: nothing to do
    if (size == 0 || start_cluster < 2) return;

    uint64_t bytes_remaining   = size;
    uint64_t bytes_per_cluster = (uint64_t)exfat_sectors_per_cluster * exfat_bytes_per_sector;
    uint32_t current_cluster   = start_cluster;
    uint64_t buffer_offset     = 0;

    while (current_cluster < 0x0FFFFFF7 && bytes_remaining > 0) {
        uint32_t lba           = exfat_cluster_to_lba(current_cluster);
        uint64_t to_read       = (bytes_remaining < bytes_per_cluster)
                                     ? bytes_remaining
                                     : bytes_per_cluster;

        // Read each sector of this cluster
        for (uint32_t s = 0; s < exfat_sectors_per_cluster && to_read > 0; s++) {
            ata_pio_read_sector(lba + s, out_buffer + buffer_offset);
            uint32_t sector_read = (to_read < exfat_bytes_per_sector)
                                       ? (uint32_t)to_read
                                       : exfat_bytes_per_sector;
            buffer_offset  += sector_read;
            bytes_remaining -= sector_read;
            to_read        -= sector_read;
        }

        // Follow the FAT chain to the next cluster
        current_cluster = exfat_next_cluster(current_cluster);
    }
}

// ==========================================
// Root Directory Debug Print
// ==========================================

void exfat_print_root_dir() {
    // Read the first sector of the root directory
    uint8_t buffer[512];
    uint32_t root_lba = exfat_cluster_to_lba(exfat_root_cluster);
    ata_pio_read_sector(root_lba, buffer);

    ExFAT_GenericEntry* entries = (ExFAT_GenericEntry*)buffer;
    int total = 512 / 32; // 16 entries per 512-byte sector

    kprintf("--- EXFAT ROOT DIRECTORY ---\n");

    for (int i = 0; i < total; ) {
        // Early break: end of directory
        if (entries[i].entry_type == EXFAT_ENTRY_EOD) break;

        // Early continue: skip non-file primaries (Bitmap, UpCase, Label, deleted)
        if (entries[i].entry_type != EXFAT_ENTRY_FILE) {
            i++;
            continue;
        }

        ExFAT_FileEntry*   file   = (ExFAT_FileEntry*)  &entries[i];
        ExFAT_StreamEntry* stream = (ExFAT_StreamEntry*) &entries[i + 1];
        ExFAT_NameEntry*   name   = (ExFAT_NameEntry*)   &entries[i + 2];

        // Decode the UTF-16LE filename to ASCII
        char ascii_name[256];
        exfat_utf16_to_ascii(name->name, stream->name_length, ascii_name);

        bool is_dir = (file->file_attributes & EXFAT_ATTR_DIRECTORY);
        kprintf("%s %s | Size: %lld bytes | Cluster: %d\n",
                is_dir ? "[DIR] " : "[FILE]",
                ascii_name,
                stream->data_length,
                stream->first_cluster);

        // Skip the whole entry set (1 primary + secondary_count secondaries)
        i += 1 + file->secondary_count;
    }

    kprintf("----------------------------\n");
}

// ==========================================
// FAT Write Operations
// ==========================================

uint32_t exfat_find_free_cluster() {
    uint8_t sector_buffer[512];
    uint32_t* fat = (uint32_t*)sector_buffer;

    // Scan up to 100 FAT sectors (same conservative limit as the FAT32 driver)
    for (uint32_t sector_offset = 0; sector_offset < 100; sector_offset++) {
        ata_pio_read_sector(exfat_fat_lba + sector_offset, sector_buffer);

        for (int i = 0; i < 128; i++) {
            if ((fat[i] & 0x0FFFFFFF) == 0x00000000) {
                uint32_t cluster = (sector_offset * 128) + i;

                // Early continue: clusters 0 and 1 are reserved
                if (cluster < 2) continue;

                return cluster;
            }
        }
    }

    return 0; // Disk full
}

void exfat_write_fat_entry(uint32_t cluster, uint32_t value) {
    // Early return: guard against reserved clusters
    if (cluster < 2) return;

    uint8_t sector_buffer[512];
    uint32_t* fat = (uint32_t*)sector_buffer;

    uint32_t fat_sector_offset   = cluster / 128;
    uint32_t fat_index_in_sector = cluster % 128;
    uint32_t target_lba          = exfat_fat_lba + fat_sector_offset;

    // Read → Modify → Write (RMW cycle, same as FAT32)
    ata_pio_read_sector(target_lba, sector_buffer);
    fat[fat_index_in_sector] = value & 0x0FFFFFFF;
    ata_pio_write_sector(target_lba, sector_buffer);
}

bool exfat_update_directory_entry(const char* filename, uint32_t start_cluster, uint64_t size) {
    // Early return: invalid filename pointer
    if (filename == nullptr) return false;

    uint8_t buffer[512];
    uint32_t root_lba = exfat_cluster_to_lba(exfat_root_cluster);
    ata_pio_read_sector(root_lba, buffer);

    ExFAT_GenericEntry* entries = (ExFAT_GenericEntry*)buffer;
    int total = 512 / 32;

    for (int i = 0; i < total; ) {
        if (entries[i].entry_type == EXFAT_ENTRY_EOD) break;

        if (entries[i].entry_type != EXFAT_ENTRY_FILE) {
            i++;
            continue;
        }

        ExFAT_FileEntry*   file   = (ExFAT_FileEntry*)  &entries[i];
        ExFAT_StreamEntry* stream = (ExFAT_StreamEntry*) &entries[i + 1];
        ExFAT_NameEntry*   name   = (ExFAT_NameEntry*)   &entries[i + 2];

        // Convert the on-disk name to ASCII for comparison
        char ascii_name[256];
        exfat_utf16_to_ascii(name->name, stream->name_length, ascii_name);

        // Simple case-insensitive comparison (ExFAT names are case-preserving)
        bool match = true;
        for (int k = 0; ascii_name[k] != '\0' || filename[k] != '\0'; k++) {
            char a = ascii_name[k];
            char b = filename[k];
            if (a >= 'a' && a <= 'z') a -= 32;
            if (b >= 'a' && b <= 'z') b -= 32;
            if (a != b) { match = false; break; }
        }

        if (match) {
            stream->first_cluster     = start_cluster;
            stream->data_length       = size;
            stream->valid_data_length = size;

            ata_pio_write_sector(root_lba, buffer);
            return true;
        }

        i += 1 + file->secondary_count;
    }

    return false; // File not found
}