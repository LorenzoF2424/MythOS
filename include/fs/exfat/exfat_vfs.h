#ifndef EXFAT_VFS_H
#define EXFAT_VFS_H

#include "fs/vfs.h"
#include "fs/exfat/exfat.h"

// ==========================================
// ExFAT File Node
// ==========================================
class ExFATFile : public VFSNode {
private:
    uint32_t start_cluster;

public:
    ExFATFile(const char* file_name, uint32_t cluster, uint64_t file_size);

    uint64_t read  (uint64_t offset, uint64_t size,       uint8_t* buffer) override;
    uint64_t write (uint64_t offset, uint64_t size, const uint8_t* buffer) override;
    VFSNode* finddir(const char* name) override;
    VFSNode* readdir(uint32_t index)  override;
};

// ==========================================
// ExFAT Directory Node
// ==========================================
class ExFATDirectory : public VFSNode {
private:
    uint32_t start_cluster;

    // Simple flat cache — avoids re-reading the disk on every lookup
    static constexpr int MAX_CHILDREN = 64;
    VFSNode* children[MAX_CHILDREN];
    int      child_count;
    bool     loaded;

    // Parses the on-disk directory cluster and populates children[]
    void load_children();

public:
    ExFATDirectory(const char* dir_name, uint32_t cluster);
    ~ExFATDirectory();

    bool     create_file(const char* new_filename) override;

    uint64_t read  (uint64_t offset, uint64_t size,       uint8_t* buffer) override;
    uint64_t write (uint64_t offset, uint64_t size, const uint8_t* buffer) override;
    VFSNode* finddir(const char* name) override;
    VFSNode* readdir(uint32_t index)  override;
};

// ==========================================
// Name Conversion Helpers
// ==========================================

// Converts a UTF-16LE ExFAT name (name_length chars) to a null-terminated ASCII string.
// Non-ASCII characters are replaced with '?'.
void exfat_utf16_to_ascii(const uint16_t* utf16, int name_length, char* out_ascii);

// Converts a null-terminated ASCII string to UTF-16LE for directory entry writing.
void exfat_ascii_to_utf16(const char* ascii, uint16_t* out_utf16, int max_chars);

// Computes the ExFAT name hash (used in the Stream Extension entry).
uint16_t exfat_name_hash(const uint16_t* name, uint8_t name_length);

#endif