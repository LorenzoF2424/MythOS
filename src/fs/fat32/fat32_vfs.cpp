#include "fs/fat32/fat32_vfs.h"
#include "drivers/storage/ata_pio.h"
#include "lib/string.h"

// External helper to calculate the physical sector of a cluster
extern uint32_t cluster_to_lba(uint32_t cluster);

// Helper to convert "SECRET  TXT" to "SECRET.TXT"
void format_fat_name(const uint8_t* fat_name, char* out_name) {
    int out_idx = 0;
    // Copy the 8-character name, ignoring padding spaces
    for (int i = 0; i < 8 && fat_name[i] != ' '; i++) {
        out_name[out_idx++] = fat_name[i];
    }
    // If there is an extension, add the dot and the extension
    if (fat_name[8] != ' ') {
        out_name[out_idx++] = '.';
        for (int i = 8; i < 11 && fat_name[i] != ' '; i++) {
            out_name[out_idx++] = fat_name[i];
        }
    }
    out_name[out_idx] = '\0';
}

// Helper function to convert "file.txt" to "FILE    TXT"
void string_to_fat_name(const char* input_name, char* fat_name) {
    // Fill with spaces by default
    memset(fat_name, ' ', 11);
    
    int i = 0;
    int j = 0;
    
    // Copy the base name (up to 8 characters)
    while (input_name[i] != '\0' && input_name[i] != '.' && j < 8) {
        char c = input_name[i];
        if (c >= 'a' && c <= 'z') c -= 32; // Convert to uppercase
        fat_name[j] = c;
        i++;
        j++;
    }
    
    // Find the extension
    while (input_name[i] != '\0' && input_name[i] != '.') {
        i++;
    }
    
    // Copy the extension (up to 3 characters)
    if (input_name[i] == '.') {
        i++;
        j = 8;
        while (input_name[i] != '\0' && j < 11) {
            char c = input_name[i];
            if (c >= 'a' && c <= 'z') c -= 32; // Convert to uppercase
            fat_name[j] = c;
            i++;
            j++;
        }
    }
}

// ==========================================
// FAT32File Implementation
// ==========================================
FAT32File::FAT32File(const char* file_name, uint32_t cluster, uint32_t file_size) {
    strcpy(this->name, file_name);
    this->flags = VFS_FILE;
    this->size = file_size;
    this->start_cluster = cluster;
}

uint64_t FAT32File::read(uint64_t offset, uint64_t read_size, uint8_t* buffer) {
    // Early return: ignore offset for now and read the raw file
    // In a complete driver, you would use the 'offset' to skip bytes
    fat32_read_file(this->start_cluster, this->size, buffer);
    return this->size; 
}

uint64_t FAT32File::write(uint64_t offset, uint64_t write_size, const uint8_t* buffer) {
    // Early return: guard against invalid parameters
    if (buffer == nullptr || write_size == 0) return 0;

    // Early return: prevent multi-cluster complexity for the first implementation
    if (write_size > 512) {
        kprintf("Error: FAT32 writes larger than 1 sector not yet supported.\n");
        return 0;
    }

    // 1. If this is a brand new file, it needs physical disk space allocated
    if (this->start_cluster == 0) {
        this->start_cluster = fat32_find_free_cluster();
        
        // Early return: the disk is completely full
        if (this->start_cluster == 0) {
            kprintf("Error: Disk is full.\n");
            return 0;
        }

        // Reserve the cluster in the FAT table (0x0FFFFFFF means End Of File)
        fat32_write_fat_entry(this->start_cluster, 0x0FFFFFFF);
    }

    // 2. Prepare a clean 512-byte buffer to avoid writing random RAM garbage to the disk
    uint8_t sector_buffer[512];
    memset(sector_buffer, 0, 512);
    memcpy(sector_buffer, buffer, write_size);

    // 3. Calculate physical address and write the actual file data
    uint32_t data_lba = cluster_to_lba(this->start_cluster);
    ata_pio_write_sector(data_lba, sector_buffer);

    // 4. Update the internal VFS node size in RAM
    if (offset + write_size > this->size) {
        this->size = offset + write_size;
    }

    // 5. CRITICAL: Update the Directory Entry on disk so the file size persists after reboot
    fat32_update_directory_entry(this->name, this->start_cluster, this->size);

    return write_size;
}

VFSNode* FAT32File::finddir(const char* name) { return nullptr; }
VFSNode* FAT32File::readdir(uint32_t index) { return nullptr; }

// ==========================================
// FAT32Directory Implementation
// ==========================================
FAT32Directory::FAT32Directory(const char* dir_name, uint32_t cluster) {
    strcpy(this->name, dir_name);
    this->flags = VFS_DIRECTORY;
    this->size = 0;
    this->start_cluster = cluster;
    this->child_count = 0;
    this->loaded = false;
}

FAT32Directory::~FAT32Directory() {
    for (int i = 0; i < child_count; i++) {
        delete children[i];
    }
}

void FAT32Directory::load_children() {
    // Early return: Only parse the disk once and cache the results
    if (loaded) return;

    uint8_t buffer[512];
    
    // Read the directory sector
    fat32_read_file(this->start_cluster, 512, buffer); 

    FAT_DirectoryEntry* entry = (FAT_DirectoryEntry*)buffer;

    for (int i = 0; i < 16; i++) {
        // Early break: 0x00 means end of directory entries
        if (entry[i].name[0] == 0x00) break; 
        
        // Early continue: Skip deleted files or Long File Names (LFN)
        if (entry[i].name[0] == 0xE5 || entry[i].attributes == 0x0F) continue; 

        char clean_name[13];
        format_fat_name(entry[i].name, clean_name);

        uint32_t cluster = ((uint32_t)entry[i].cluster_high << 16) | entry[i].cluster_low;

        // Instantiate the correct VFSNode type
        if (entry[i].attributes & 0x10) {
            children[child_count] = new FAT32Directory(clean_name, cluster);
        } else {
            children[child_count] = new FAT32File(clean_name, cluster, entry[i].size);
        }
        child_count++;
    }
    loaded = true;
}

VFSNode* FAT32Directory::readdir(uint32_t index) {
    load_children();
    
    // Early return: index out of bounds
    if (index >= child_count) return nullptr;
    
    return children[index];
}

VFSNode* FAT32Directory::finddir(const char* target_name) {
    load_children();
    for (int i = 0; i < child_count; i++) {
        if (strcmp(children[i]->name, target_name) == 0) {
            return children[i];
        }
    }
    return nullptr;
}

uint64_t FAT32Directory::read(uint64_t offset, uint64_t read_size, uint8_t* buffer) { return 0; }
uint64_t FAT32Directory::write(uint64_t offset, uint64_t write_size, const uint8_t* buffer) { return 0; }

bool FAT32Directory::create_file(const char* new_filename) {
    uint8_t buffer[512];
    
    // 1. Read the current directory sector from the disk
    fat32_read_file(this->start_cluster, 512, buffer);
    FAT_DirectoryEntry* entries = (FAT_DirectoryEntry*)buffer;
    
    int free_slot = -1;
    
    // 2. Scan the 16 available slots in this sector (512 / 32 = 16)
    for (int i = 0; i < 16; i++) {
        // Early break: 0x00 means we reached the unallocated end of the directory
        if (entries[i].name[0] == 0x00) {
            free_slot = i;
            break;
        }
        // Early break: 0xE5 means the file was deleted, we can reuse this slot
        if (entries[i].name[0] == 0xE5) {
            free_slot = i;
            break;
        }
    }
    
    // Early return: no free slots available in the current sector
    if (free_slot == -1) {
        kprintf("Error: Directory is full. Cannot create file.\n");
        return false;
    }
    
    // 3. Format the filename to the 8.3 FAT standard
    char fat_name[11];
    string_to_fat_name(new_filename, fat_name);
    
    // 4. Populate the 32-byte slot
    memcpy(entries[free_slot].name, fat_name, 11);
    entries[free_slot].attributes = 0x20; // 0x20 = Archive (Standard File)
    entries[free_slot].reserved = 0;
    entries[free_slot].creation_time_tenths = 0;
    entries[free_slot].creation_time = 0;
    entries[free_slot].creation_date = 0;
    entries[free_slot].last_access_date = 0;
    entries[free_slot].cluster_high = 0; // No data assigned yet
    entries[free_slot].last_write_time = 0;
    entries[free_slot].last_write_date = 0;
    entries[free_slot].cluster_low = 0;  // No data assigned yet
    entries[free_slot].size = 0;         // Initial size is 0
    
    // 5. Calculate LBA and flush the updated sector to the physical disk
    uint32_t lba = cluster_to_lba(this->start_cluster);
    ata_pio_write_sector(lba, buffer);
    
    // 6. Update the VFS memory structure so the new file is visible immediately
    if (child_count < 256) {
        children[child_count] = new FAT32File(new_filename, 0, 0);
        child_count++;
    }
    
    return true;
}