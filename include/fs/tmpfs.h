#ifndef TMPFS_H
#define TMPFS_H

#include "vfs.h"
#include "gnu_utils/string.h"

// --- FILE IMPLEMENTATION ---
class TmpFSFile : public VFSNode {
private:
    uint8_t* data_buffer; // Pointer to the memory area containing the data
    uint64_t capacity;    // How much RAM we have allocated so far
    
public:
    TmpFSFile(const char* filename);
    ~TmpFSFile() override;

    uint64_t read(uint64_t offset, uint64_t size, uint8_t* buffer) override;
    uint64_t write(uint64_t offset, uint64_t size, const uint8_t* buffer) override;
    
    // Files do not have children, so these return null/empty
    VFSNode* finddir(const char* name) override { return nullptr; }
    VFSNode* readdir(uint32_t index) override { return nullptr; }
};

// --- DIRECTORY IMPLEMENTATION ---
class TmpFSDirectory : public VFSNode {
private:
    VFSNode** children; // Array of pointers to children (files or other dirs)
    int child_count;    // Number of active children
    int capacity;       // How many children the array can currently hold

public:
    TmpFSDirectory(const char* dirname);
    ~TmpFSDirectory() override;

    // A directory cannot be read or written to like a raw text file
    uint64_t read(uint64_t offset, uint64_t size, uint8_t* buffer) override { return 0; }
    uint64_t write(uint64_t offset, uint64_t size, const uint8_t* buffer) override { return 0; }
    
    // Directory specific operations
    VFSNode* finddir(const char* name) override;
    VFSNode* readdir(uint32_t index) override;
    
    // We need a way to populate the directory!
    bool add_child(VFSNode* child);
    bool remove_child(const char* name);
};

#endif // TMPFS_H