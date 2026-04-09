#include "cli/commands.h"
#include "fs/vfs.h"
#include "fs/tmpfs.h"

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

    TmpFSDirectory* current_tmp_dir = (TmpFSDirectory*)vfs_current_dir;
    VFSNode* new_file = new TmpFSFile(argv[1]);

    // Early return: out of memory
    if (!new_file) {
        kprintf("Error: Out of memory.\n");
        return true;
    }

    // Try to add the child file
    if (!current_tmp_dir->add_child(new_file)) {
        kprintf("Error: Could not create file '%s'. It might already exist.\n", argv[1]);
        delete new_file;
        return true;
    }
    
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
}