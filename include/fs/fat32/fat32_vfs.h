#ifndef FAT32_VFS_H
#define FAT32_VFS_H

#include "fs/vfs.h"
#include "fs/fat32/fat32.h" // To access our raw reading functions

// ==========================================
// FAT32 File Node
// ==========================================
class FAT32File : public VFSNode {
private:
    uint32_t start_cluster;

public:
    FAT32File(const char* file_name, uint32_t cluster, uint32_t file_size);
    
    // Fulfilling the VFS Contract
    uint64_t read(uint64_t offset, uint64_t size, uint8_t* buffer) override;
    uint64_t write(uint64_t offset, uint64_t size, const uint8_t* buffer) override;
    VFSNode* finddir(const char* name) override;
    VFSNode* readdir(uint32_t index) override;
};

// ==========================================
// FAT32 Directory Node
// ==========================================
class FAT32Directory : public VFSNode {
private:
    uint32_t start_cluster;
    
    // Simple cache for children nodes to avoid reading the disk constantly
    VFSNode* children[16]; 
    int child_count;
    bool loaded;

    // Helper to read the disk and populate the children array
    void load_children();

public:
    FAT32Directory(const char* dir_name, uint32_t cluster);
    ~FAT32Directory();

    // Creates a new empty file entry on the physical disk
    bool create_file(const char* new_filename) override;

    // Fulfilling the VFS Contract
    uint64_t read(uint64_t offset, uint64_t size, uint8_t* buffer) override;
    uint64_t write(uint64_t offset, uint64_t size, const uint8_t* buffer) override;
    VFSNode* finddir(const char* name) override;
    VFSNode* readdir(uint32_t index) override;
};

void format_fat_name(const uint8_t* fat_name, char* out_name);

void string_to_fat_name(const char* input_name, char* fat_name);


#endif