#include "fs/vfs.h"
#include "fs/tmpfs.h"
#include "lib/string.h"
#include "fs/fat32/fat32.h"


// ==========================================
// Mount Points Management
// ==========================================


// Instead of a static array, we use a pointer
MountPoint* vfs_mounts = nullptr; 

uint8_t vfs_mount_count = 0;
const uint8_t VFS_MAX_MOUNTS = 16; 

void vfs_mount(const char* target_name, VFSNode* fs_root_node) {
    // Early return: check if the array was properly allocated
    if (vfs_mounts == nullptr) {
        kprintf("[VFS] Error: Mount points memory not allocated.\n");
        return;
    }

    // Early return: maximum mount points reached
    if (vfs_mount_count >= VFS_MAX_MOUNTS) {
        kprintf("[VFS] Error: Maximum mount points reached.\n");
        return;
    }

    // Copy the target name and link the node
    strcpy(vfs_mounts[vfs_mount_count].name, target_name);
    vfs_mounts[vfs_mount_count].node = fs_root_node;
    
    vfs_mount_count++;
    
    kprintf("[VFS] Mounted device to /%s successfully.\n", target_name);
}
// ==========================================
// Global Variables Definition
// ==========================================
VFSNode* vfs_root = nullptr;
VFSNode* vfs_current_dir = nullptr;
char vfs_current_path[4096];
char vfs_display_path[4096];

// The display name of the root directory (Used ONLY for frontend printing)
const char* vfs_root_display_name = "root";


// ==========================================
// VFS Initialization
// ==========================================
void init_vfs() {
    kprintf("[VFS] Initializing Virtual File System...\n");

    // 0. Allocate exactly 1 memory block (4KB) for our Mount Points array
    vfs_mounts = (MountPoint*)pmm_alloc_blocks(1);

    // Early return/Panic: Kernel Out of Memory
    if (vfs_mounts == nullptr) {
        kprintf("[VFS] PANIC: Failed to allocate memory for mount points!\n");
        while(1); // Halt the system
    }

    // Clear the path buffer securely
    strinit(vfs_current_path, 4096);

    // 1. Create the root directory using the BACKEND standard ("/")
    vfs_root = new TmpFSDirectory("/");

    // Early return/Panic: Kernel Out of Memory
    if (vfs_root == nullptr) {
        kprintf("[VFS] PANIC: Failed to create root directory!\n");
        while(1); // Halt the system
    }

    // 2. Set the Current Working Directory to root
    vfs_current_dir = vfs_root;

    // 3. Initialize the backend path string to standard POSIX root ("/")
    strcpy(vfs_current_path, "/");

    
    kprintf("[VFS] Root filesystem mounted successfully.\n");
}

// ==========================================
// Path Resolution
// ==========================================
void vfs_resolve_path(const char* input_path, char* output_path) {
    // Early return: invalid pointers
    if (input_path == nullptr || output_path == nullptr) return;

    char work_buffer[4352];
    int w_idx = 0;

    // If path is relative (doesn't start with '/'), prepend the current working directory
    if (input_path[0] != '/') {
        for (int i = 0; vfs_current_path[i] != '\0'; i++) {
            work_buffer[w_idx++] = vfs_current_path[i];
        }
        // Ensure there is a trailing slash before appending the new part
        if (w_idx > 0 && work_buffer[w_idx - 1] != '/') {
            work_buffer[w_idx++] = '/';
        }
    }

    // Append the input path provided by the user
    for (int i = 0; input_path[i] != '\0'; i++) {
        work_buffer[w_idx++] = input_path[i];
    }
    work_buffer[w_idx] = '\0';

    // Stack to keep track of valid directory levels (max 32 levels deep)
    char stack[32][64]; 
    int top = 0;

    // Tokenize the dirty path using '/'
    char* token = strtok(work_buffer, "/");
    while (token != nullptr) {
        if (strcmp(token, ".") == 0) {
            // Do nothing, stay in current directory
        } else if (strcmp(token, "..") == 0) {
            // Go back one level (pop from stack)
            if (top > 0) top--;
        } else if (token[0] != '\0') {
            // Go down one level (push to stack)
            strcpy(stack[top], token);
            top++;
        }
        token = strtok(nullptr, "/");
    }

    // Reconstruct the clean absolute path
    output_path[0] = '/';
    output_path[1] = '\0';
    int out_idx = 1;

    for (int i = 0; i < top; i++) {
        for (int j = 0; stack[i][j] != '\0'; j++) {
            output_path[out_idx++] = stack[i][j];
        }
        // Add a slash between elements, but not at the very end
        if (i < top - 1) {
            output_path[out_idx++] = '/';
        }
    }
    output_path[out_idx] = '\0';
}

// ==========================================
// Node Traversal
// ==========================================
VFSNode* vfs_get_node_by_path(const char* absolute_path) {
    if (absolute_path == nullptr || vfs_root == nullptr) return nullptr;

    // 1. Handle Root Request
    if (strcmp(absolute_path, "/") == 0) return vfs_root;

    char path_copy[256];
    strcpy(path_copy, absolute_path);
    char* token = strtok(path_copy, "/");

    VFSNode* current = nullptr;

    // 2. Check if the first part is a MOUNT POINT
    if (token != nullptr && vfs_mounts != nullptr) {
        for (int i = 0; i < vfs_mount_count; i++) {
            if (strcmp(token, vfs_mounts[i].name) == 0) {
                // We matched "/hdd" -> Switch driver to FAT32
                current = vfs_mounts[i].node;
                token = strtok(nullptr, "/"); 
                break;
            }
        }
    }

    // 3. If not a mount point, start from RAM root
    if (current == nullptr) {
        current = vfs_root;
        // token is still the first part of the path
    } else {
        // If it WAS a mount point and there's nothing left, return the mount root
        if (token == nullptr) return current;
    }

    // 4. Walk the remaining tree (either RAM or FAT32)
    while (token != nullptr) {
        if (current->flags != VFS_DIRECTORY) return nullptr;

        current = current->finddir(token);
        if (current == nullptr) return nullptr;

        token = strtok(nullptr, "/");
    }

    return current;
}

// ==========================================
// Frontend Display Path Formatting
// ==========================================
void vfs_get_display_path(char* output_buffer) {
    // Early return: safety check
    if (output_buffer == nullptr) return;

    // 1. Start with the root display name (e.g., "root")
    strcpy(output_buffer, vfs_root_display_name);

    // 2. Early return: if we are exactly at the backend root "/", 
    // we just want "root", so we are done.
    if (strcmp(vfs_current_path, "/") == 0) return;

    // 3. If we are in a subdirectory (e.g., "/a"), vfs_current_path starts with '/'.
    // We append it directly to "root". 
    // Result: "root" + "/a" = "root/a"
    size_t root_len = strlen(vfs_root_display_name);
    strcpy(output_buffer + root_len, vfs_current_path);
}



void vfs_print_prompt() {
    // Local buffer for the formatted path (PATH_MAX + prefix size)
    char display_path[4352]; 
    
    // Format the path
    vfs_get_display_path(display_path);
    
    // Print the final result to the terminal
    kprintf("%s>", display_path);
}