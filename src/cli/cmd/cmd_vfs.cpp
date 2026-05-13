#include "cli/commands.h"
#include "fs/vfs.h"
#include "fs/tmpfs.h"
#include "fs/fat32/fat32_vfs.h"

// ==========================================
// VFS Command Implementations
// ==========================================

int8_t cmd_dir(int argc, char** argv) {
    // Early return: safety check for current directory
    if (vfs_current_dir == nullptr) return true;

    char display_path[4096];
    vfs_get_display_path(display_path);
    kprintf("\nDirectory listing for %s\n\n", display_path);

    uint32_t index = 0;
    VFSNode* child = vfs_current_dir->readdir(index);
    int total_items = 0;

    // Iterate through directory contents
    while (child != nullptr) {
        if (child->flags == VFS_DIRECTORY) {
            kprintf("  <DIR>    %s\n", child->name);
        } else {
            kprintf("  <FILE>   %s (%d bytes)\n", child->name, (int)child->size);
        }
        index++;
        total_items++;
        child = vfs_current_dir->readdir(index);
    }
    kprintf("\n  Total: %d items\n", total_items);
    return true;
}

int8_t cmd_mkdir(int argc, char** argv) {
    // Early return: check arguments
    if (argc < 2) return 2;
    
    // Early return: check valid directory state
    if (vfs_current_dir == nullptr || vfs_current_dir->flags != VFS_DIRECTORY) {
        kprintf("Error: Current path is not a valid directory.\n");
        return true;
    }

    TmpFSDirectory* current_tmp_dir = (TmpFSDirectory*)vfs_current_dir;
    VFSNode* new_dir = new TmpFSDirectory(argv[1]);

    // Early return: out of memory
    if (!new_dir) {
        kprintf("Error: Out of memory.\n");
        return true;
    }

    // Try to add the child directory
    if (!current_tmp_dir->add_child(new_dir)) {
        kprintf("Error: Could not create directory '%s'. It might already exist.\n", argv[1]);
        delete new_dir;
        return true;
    }
    
    kprintf("Directory '%s' created successfully.\n", argv[1]);
    return true;
}

int8_t cmd_touch(int argc, char** argv) {
    // Early return: check arguments
    if (argc < 2) return 2;
    
    // Early return: check valid directory state
    if (vfs_current_dir == nullptr || vfs_current_dir->flags != VFS_DIRECTORY) {
         kprintf("Error: Current path is not a valid directory.\n");
         return true;
    }

    // Check if we are inside the FAT32 mount point
    if (strncmp(vfs_current_path, "/hdd", 4) == 0) {
        
        // --- FAT32 File System Logic ---
        FAT32Directory* current_fat_dir = (FAT32Directory*)vfs_current_dir;
        
        // Early return: disk write failed
        if (!current_fat_dir->create_file(argv[1])) {
            // The create_file method already prints the specific error
            return true; 
        }

    } else {
        
        // --- TmpFS (RAM Disk) Logic ---
        TmpFSDirectory* current_tmp_dir = (TmpFSDirectory*)vfs_current_dir;
        VFSNode* new_file = new TmpFSFile(argv[1]);

        // Early return: out of memory allocating the VFS node
        if (!new_file) {
            kprintf("Error: Out of memory.\n");
            return true;
        }

        // Early return: failed to add child to the RAM directory
        if (!current_tmp_dir->add_child(new_file)) {
            kprintf("Error: Could not create file '%s'. It might already exist.\n", argv[1]);
            delete new_file;
            return true;
        }
    }
    
    // Success message for both file systems
    kprintf("File '%s' created successfully.\n", argv[1]);
    return true;
}

int8_t cmd_cd(int argc, char** argv) {
    // Early return: check arguments
    if (argc < 2) return 2;
    
    // Early return: check VFS initialization
    if (vfs_root == nullptr) {
        kprintf("Error: VFS is not initialized.\n");
        return true;
    }

    char target_path[4096];
    vfs_resolve_path(argv[1], target_path);

    VFSNode* target_node = vfs_get_node_by_path(target_path);
    
    // Early return: invalid target or not a directory
    if (!target_node || target_node->flags != VFS_DIRECTORY) {
        kprintf("Error: Directory '%s' not found.\n", argv[1]);
        return true;
    }

    // Update current VFS state
    vfs_current_dir = target_node;
    strcpy(vfs_current_path, target_path);
    return true;
}

int8_t cmd_ls(int argc, char** argv) {
    // Early return: safety check for current directory
    if (vfs_current_dir == nullptr) {
        kprintf("Error: Current directory is invalid.\n");
        return true; 
    }

    uint32_t index = 0;
    VFSNode* child = vfs_current_dir->readdir(index);

    // Iterate through directory contents and print names separated by a space
    while (child != nullptr) {
        kprintf("%s ", child->name);
        
        // Move to the next item
        index++;
        child = vfs_current_dir->readdir(index);
    }
    
    // Print a final newline to keep the terminal prompt clean
    kprintf("\n");

    return true;
}

int8_t cmd_cat(int argc, char** argv) {
    // Early return: check if the user provided a filename
    if (argc < 2) return 2; // Return standard usage error

    // Early return: check if the Virtual File System is ready
    if (vfs_root == nullptr) {
        kprintf("Error: VFS is not initialized.\n");
        return true;
    }

    // Resolve the path to handle relative names like "SECRET.TXT"
    char target_path[4096];
    vfs_resolve_path(argv[1], target_path);

    // Retrieve the node from the VFS tree
    VFSNode* target_node = vfs_get_node_by_path(target_path);

    // Early return: file not found or invalid
    if (!target_node) {
        kprintf("Error: File '%s' not found.\n", argv[1]);
        return true;
    }

    // Early return: cannot display the content of a directory
    if (target_node->flags == VFS_DIRECTORY) {
        kprintf("Error: '%s' is a directory.\n", argv[1]);
        return true;
    }

    uint64_t file_size = target_node->size;
    if (file_size == 0) return true; // Nothing to print

    /* * ANTI-CORRUPTION STRATEGY:
     * We allocate more space than strictly required and align it to 32 bytes.
     * This creates a physical "padding zone" between the file data and the 
     * Heap metadata (SlabBlock canary), preventing off-by-one errors in 
     * the driver from corrupting the Slab magic value.
     */
    size_t safe_alloc_size = (file_size + 64) & ~31; 
    char* buffer = (char*)malloc(safe_alloc_size);

    // Early return: check if memory allocation succeeded
    if (!buffer) {
        kprintf("Error: Heap is full or memory request is too large.\n");
        return true;
    }

    // Zero out the entire buffer to prevent data leaks and ensure null termination
    memset(buffer, 0, safe_alloc_size);

    /* * Execute the read operation.
     * Even if the driver writes slightly more than file_size (e.g., an extra \0),
     * it will fall into our safety padding instead of hitting the Heap metadata.
     */
    target_node->read(0, file_size, (uint8_t*)buffer);
    
    // Explicitly null-terminate exactly where the file ends
    buffer[file_size] = '\0';

    // Print the file content to the terminal
    kprintf("%s\n", buffer);

    // Free the allocated memory. The canary check in kheap.cpp will now pass.
    free(buffer);

    return true;
}

int8_t cmd_write(int argc, char** argv) {
    // Early return: check arguments. We need command, filename, and text.
    if (argc < 3) {
        kprintf("Usage: write <filename> <text...>\n");
        return 2;
    }

    // Early return: check valid directory state
    if (vfs_current_dir == nullptr) {
        kprintf("Error: No current directory.\n");
        return true;
    }

    const char* target_filename = argv[1];
    VFSNode* target_node = vfs_current_dir->finddir(target_filename);

    // Auto-create the file if it does not exist on the disk/RAM
    if (!target_node) {
        if (!vfs_current_dir->create_file(target_filename)) {
            kprintf("Error: Could not create file '%s' on disk.\n", target_filename);
            return true;
        }
        // Fetch the newly created node
        target_node = vfs_current_dir->finddir(target_filename);
    }

    // Early return: cannot write to a directory or invalid node
    if (target_node == nullptr || target_node->flags == VFS_DIRECTORY) {
        kprintf("Error: '%s' is an invalid target or a directory.\n", target_filename);
        return true;
    }

    // Reconstruct the text to write from argv[2] onwards
    char text_buffer[1024];
    memset(text_buffer, 0, 1024);
    size_t buffer_pos = 0;

    for (int i = 2; i < argc; i++) {
        const char* word = argv[i];
        
        // Copy the current word into the buffer
        while (*word != '\0' && buffer_pos < 1023) {
            text_buffer[buffer_pos] = *word;
            buffer_pos++;
            word++;
        }
        
        // Add a space between words (but not after the very last word)
        if (i < argc - 1 && buffer_pos < 1023) {
            text_buffer[buffer_pos] = ' ';
            buffer_pos++;
        }
    }

    // Early return: nothing to write (edge case protection)
    if (buffer_pos == 0) return true;

    /* * EXECUTE THE WRITE
     * Polymorphism handles routing this to FAT32File::write or TmpFSFile::write
     */
    uint64_t bytes_written = target_node->write(0, buffer_pos, (const uint8_t*)text_buffer);

    if (bytes_written > 0) {
        kprintf("Successfully wrote %d bytes to '%s'.\n", (int)bytes_written, target_filename);
    } else {
        kprintf("Error: Failed to write data to storage.\n");
    }

    return true;
}

// ==========================================
// Module Registration
// ==========================================

void register_vfs_commands() {
    register_command("dir", cmd_dir, "Lists files and directories", "");
    register_command("ls", cmd_ls, "Lists files and directories (compact format)", "");
    register_command("mkdir", cmd_mkdir, "Creates a new directory", "[name]");
    register_command("md", cmd_mkdir, "Creates a new directory (alias for mkdir)", "[name]");
    register_command("touch", cmd_touch, "Creates a new empty file", "[name]");
    register_command("cd", cmd_cd, "Changes current directory", "[path]");
    register_command("cat", cmd_cat, "Displays the content of a file", "[filename]");
    register_command("write", cmd_write, "Writes text to a file (creates if missing)", "<file> <text...>");
}