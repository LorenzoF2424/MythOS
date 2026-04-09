#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include "term/kprintf.h"
#include "gnu_utils/string.h"

#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02

class VFSNode {
public:
    char name[256];     // Name of the file or directory
    uint32_t flags;     // VFS_FILE or VFS_DIRECTORY
    uint64_t size;      // Size in bytes

    virtual ~VFSNode() = default;

    // --- The "Contract" that every FS must fulfill ---
    virtual uint64_t read(uint64_t offset, uint64_t size, uint8_t* buffer) = 0;
    virtual uint64_t write(uint64_t offset, uint64_t size, const uint8_t* buffer) = 0;
    virtual VFSNode* finddir(const char* name) = 0;
    virtual VFSNode* readdir(uint32_t index) = 0; 
};

// ==========================================
// VFS Subsystem Global State
// ==========================================

// Global pointers for the Root and the Current Working Directory (CWD)
extern VFSNode* vfs_root;
extern VFSNode* vfs_current_dir;
extern char vfs_current_path[4096];
extern char *root_name;

// Initializes the VFS subsystem and mounts the root filesystem
void init_vfs();

// Cleans a dirty path (handling . and ..) and returns a neat absolute path
void vfs_resolve_path(const char* input_path, char* output_path);

// Traverses the tree from the root to find a specific node by its absolute path
VFSNode* vfs_get_node_by_path(const char* absolute_path);

// Returns the formatted path for the terminal display (e.g., "root/folder")
void vfs_get_display_path(char* output_buffer);

// Prints the formatted terminal prompt (e.g., "root/folder> ")
void vfs_print_prompt();



#endif 