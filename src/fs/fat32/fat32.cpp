#include "fs/fat32/fat32.h"
#include "drivers/storage/ata_pio.h"
// Ensure you include your kprintf header
// #include "cli/CLI.h" 

// ==========================================
// FAT32 Internal State
// ==========================================
uint32_t fat_start_lba;
uint32_t data_start_lba;
uint8_t  sectors_per_cluster;
uint32_t root_dir_cluster;

// ==========================================
// Driver Implementation
// ==========================================

void fat32_init(uint32_t partition_lba_start) {
    uint8_t buffer[512];

    // 1. Read the Volume Boot Record (first sector of the FAT32 partition)
    ata_pio_read_sector(partition_lba_start, buffer);

    // 2. Cast the raw bytes to our BPB structure
    FAT32_BPB* bpb = (FAT32_BPB*)buffer;

    // Early return: check the boot sector signature (always 0xAA55 at the end)
    if (buffer[510] != 0x55 || buffer[511] != 0xAA) {
        kprintf("Error: Invalid FAT32 Boot Sector signature.\n");
        return;
    }

    // Early return: double check it's actually FAT32
    // Some formats pad this field with spaces, so checking the first 5 chars is enough
    if (bpb->fs_type[0] != 'F' || bpb->fs_type[1] != 'A' || bpb->fs_type[2] != 'T') {
        kprintf("Error: Partition is not formatted as FAT32.\n");
        return;
    }

    // 3. Save the crucial information to navigate the disk later
    sectors_per_cluster = bpb->sectors_per_cluster;
    root_dir_cluster = bpb->root_cluster;

    // Calculate where the FAT tables start
    // Formula: Partition Start + Reserved Sectors
    fat_start_lba = partition_lba_start + bpb->reserved_sectors;

    // Calculate where the actual file/folder data starts
    // Formula: FAT Start + (Number of FATs * Size of one FAT)
    uint32_t fat_size = bpb->sectors_per_fat_32;
    data_start_lba = fat_start_lba + (bpb->fat_count * fat_size);

    // 4. Print the volume info to confirm success
    kprintf("\n=== FAT32 Volume Initialized ===\n");
    
    kprintf("Volume Label: ");
    for(int i = 0; i < 11; i++) {
        kprintf("%c", bpb->volume_label[i]);
    }
    kprintf("\n");

    kprintf("Bytes per Sector : %d\n", bpb->bytes_per_sector);
    kprintf("Sectors/Cluster  : %d\n", sectors_per_cluster);
    kprintf("FAT Start LBA    : %d\n", fat_start_lba);
    kprintf("Data Start LBA   : %d\n", data_start_lba);
    kprintf("Root Dir Cluster : %d\n", root_dir_cluster);
    kprintf("================================\n\n");


    // 1. Create the Virtual VFS Node for the FAT32 Root
    // This uses the bridge class FAT32Directory we defined earlier
    FAT32Directory* fat32_root_node = new FAT32Directory("hdd", root_dir_cluster);

    // 2. Register this node as "/hdd" in our Mount Table
    // This is what makes 'cd hdd' work!
    vfs_mount("hdd", fat32_root_node);

    // Optional: Keep the print for debugging if you want
    // fat32_print_root_dir();
}

// ==========================================
// Helper Functions
// ==========================================

// Converts a FAT cluster number to a physical disk LBA sector
uint32_t cluster_to_lba(uint32_t cluster) {
    // Early return/fallback: Clusters 0 and 1 are reserved, data starts at cluster 2
    if (cluster < 2) return data_start_lba;
    
    return data_start_lba + ((cluster - 2) * sectors_per_cluster);
}

// ==========================================
// Directory Operations
// ==========================================

void fat32_print_root_dir() {
    uint8_t buffer[512];
    
    // 1. Calculate where the root directory physically starts
    uint32_t root_lba = cluster_to_lba(root_dir_cluster);
    
    // 2. Read the first sector of the root directory
    ata_pio_read_sector(root_lba, buffer);
    
    // 3. Map our 32-byte structure onto the 512-byte buffer
    // 512 / 32 = 16 entries maximum in a single sector
    FAT_DirectoryEntry* entry = (FAT_DirectoryEntry*)buffer;
    
    kprintf("--- ROOT DIRECTORY CONTENTS ---\n");
    
    for (int i = 0; i < 16; i++) {
        // Early break: 0x00 means we reached the end of the directory list
        if (entry[i].name[0] == 0x00) break;
        
        // Early continue: 0xE5 means the file was deleted. 
        // 0x0F means it's a Long File Name fragment (we skip these for now to keep it simple)
        if (entry[i].name[0] == 0xE5 || entry[i].attributes == 0x0F) continue;
        
        // Is it a directory or a file?
        bool is_dir = (entry[i].attributes & 0x10);
        
        // Print the name (FAT pads the 11 characters with spaces)
        kprintf("%s ", is_dir ? "[DIR]" : "[FILE]");
        for (int j = 0; j < 11; j++) {
            kprintf("%c", entry[i].name[j]);
        }
        
        // Print size and starting cluster
        uint32_t first_cluster = ((uint32_t)entry[i].cluster_high << 16) | entry[i].cluster_low;
        kprintf(" | Size: %d bytes | Start Cluster: %d\n", entry[i].size, first_cluster);
    }
    
    kprintf("-------------------------------\n");
}


// ==========================================
// File Reading Operations
// ==========================================

void fat32_read_file(uint32_t start_cluster, uint32_t size, uint8_t* out_buffer) {
    // Early return: no data to read
    if (size == 0) return;

    // Convert the starting cluster to a physical disk sector
    uint32_t lba = cluster_to_lba(start_cluster);

    // For this initial test, we only read the first sector (512 bytes).
    // A fully mature driver would read the FAT table to follow fragmented files.
    ata_pio_read_sector(lba, out_buffer);

    
}

// ==========================================
// FAT32 Write Operations
// ==========================================

uint32_t fat32_find_free_cluster() {
    uint8_t sector_buffer[512];
    uint32_t* fat_entries = (uint32_t*)sector_buffer;
    
    // In FAT32, each 512-byte sector holds 128 entries (32 bits each)
    // We scan the first few sectors of the FAT table for a free spot (0x00000000)
    // Note: A real OS would cache this or read the FSInfo sector, but scanning is safe.
    for (uint32_t sector_offset = 0; sector_offset < 100; sector_offset++) {
        
        ata_pio_read_sector(fat_start_lba + sector_offset, sector_buffer);
        
        for (int i = 0; i < 128; i++) {
            // Early return: 0x00000000 means the cluster is unallocated/free
            if (fat_entries[i] == 0x00000000) {
                // Calculate the absolute cluster number
                // Cluster 0 and 1 are reserved, so absolute index matches physical layout
                uint32_t absolute_cluster = (sector_offset * 128) + i;
                
                // Early continue: skip reserved clusters 0 and 1
                if (absolute_cluster < 2) continue; 
                
                return absolute_cluster;
            }
        }
    }
    
    // Return 0 if the disk is completely full (or we scanned too far)
    return 0; 
}

void fat32_write_fat_entry(uint32_t cluster, uint32_t value) {
    // Early return: guard against invalid cluster numbers
    if (cluster < 2) return;

    uint8_t sector_buffer[512];
    uint32_t* fat_entries = (uint32_t*)sector_buffer;
    
    // Calculate exactly which sector of the FAT holds our target cluster
    uint32_t fat_sector_offset = cluster / 128;
    uint32_t fat_index_in_sector = cluster % 128;
    
    uint32_t target_lba = fat_start_lba + fat_sector_offset;
    
    // 1. Read the current FAT sector into memory
    ata_pio_read_sector(target_lba, sector_buffer);
    
    // 2. Modify only our specific 32-bit entry, preserving the others
    // We bitwise AND with 0x0FFFFFFF because the top 4 bits in FAT32 are reserved
    fat_entries[fat_index_in_sector] = value & 0x0FFFFFFF;
    
    // 3. Write the modified sector back to the physical disk
    ata_pio_write_sector(target_lba, sector_buffer);
}

bool fat32_update_directory_entry(const char* filename, uint32_t start_cluster, uint32_t size) {
    // Early return: invalid filename pointer
    if (filename == nullptr) return false;

    uint8_t sector_buffer[512];
    FAT_DirectoryEntry* entries = (FAT_DirectoryEntry*)sector_buffer;
    
    // For this implementation, we assume the root directory is within the first cluster.
    // Convert the root directory cluster to a physical LBA
    uint32_t root_lba = cluster_to_lba(root_dir_cluster); 
    
    // Read the root directory sector
    ata_pio_read_sector(root_lba, sector_buffer);
    
    // Convert the user's string "FILE.TXT" to FAT standard "FILE    TXT"
    char fat_name[11];
    string_to_fat_name(filename, fat_name);
    
    // Scan the 16 slots in the 512-byte sector
    for (int i = 0; i < 16; i++) {
        // Early break: reached the end of allocated entries
        if (entries[i].name[0] == 0x00) break;
        
        // Check if this slot matches our target file
        bool match = true;
        for (int k = 0; k < 11; k++) {
            if (entries[i].name[k] != fat_name[k]) {
                match = false;
                break;
            }
        }
        
        if (match) {
            // Update the cluster address (split into High and Low 16-bit chunks)
            entries[i].cluster_high = (uint16_t)((start_cluster >> 16) & 0xFFFF);
            entries[i].cluster_low  = (uint16_t)(start_cluster & 0xFFFF);
            
            // Update the file size
            entries[i].size = size;
            
            // Flush the changes permanently to the disk!
            ata_pio_write_sector(root_lba, sector_buffer);
            return true;
        }
    }
    
    return false; // File not found in the directory sector
}