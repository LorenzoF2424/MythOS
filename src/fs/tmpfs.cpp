#include "fs/tmpfs.h"


// Note: Ensure you have memset available in your kernel (or string.cpp).
// It is required to zero-initialize the allocated RAM buffers securely.
extern "C" void* memset(void* dest, int c, size_t n);

// ==========================================
// TmpFSFile Implementation
// ==========================================

TmpFSFile::TmpFSFile(const char* filename) {
    // Safely copy the filename
    size_t i = 0;
    while (filename[i] != '\0' && i < 255) {
        this->name[i] = filename[i];
        i++;
    }
    this->name[i] = '\0';
    
    // Initialize VFSNode base properties
    this->flags = VFS_FILE;
    this->size = 0;
    
    // Initialize TmpFSFile specific properties
    this->capacity = 0;
    this->data_buffer = nullptr;
}

TmpFSFile::~TmpFSFile() {
    // Early return if there is no buffer to free
    if (this->data_buffer == nullptr) return;
    
    delete[] this->data_buffer;
}

uint64_t TmpFSFile::read(uint64_t offset, uint64_t read_size, uint8_t* buffer) {
    // Early return: if the offset is beyond EOF or buffer is null
    if (offset >= this->size) return 0;
    if (buffer == nullptr) return 0;
    
    // Calculate how many bytes we can actually read without overflowing
    uint64_t available_bytes = this->size - offset;
    uint64_t bytes_to_read = (read_size < available_bytes) ? read_size : available_bytes;
    
    // Copy data from our RAM buffer to the user's buffer using your memmove
    memmove(buffer, this->data_buffer + offset, bytes_to_read);
    
    return bytes_to_read;
}

uint64_t TmpFSFile::write(uint64_t offset, uint64_t write_size, const uint8_t* buffer) {
    // Early return: invalid buffer or zero write size
    if (buffer == nullptr || write_size == 0) return 0;

    uint64_t required_capacity = offset + write_size;
    
    // If the file needs to grow beyond its current RAM allocation
    if (required_capacity > this->capacity) {
        // Allocate a bit more than strictly necessary to prevent frequent reallocations
        uint64_t new_capacity = required_capacity + 512; 
        
        uint8_t* new_buffer = new uint8_t[new_capacity];
        
        // Early return on Kernel Out of Memory
        if (new_buffer == nullptr) return 0; 
        
        // Zero out the new memory
        memset(new_buffer, 0, new_capacity);
        
        // If we had old data, copy it over to the new buffer and free the old one
        if (this->data_buffer != nullptr) {
            memmove(new_buffer, this->data_buffer, this->size);
            delete[] this->data_buffer;
        }
        
        // Switch to the new buffer
        this->data_buffer = new_buffer;
        this->capacity = new_capacity;
    }
    
    // Write the new data into the buffer using your memmove
    memmove(this->data_buffer + offset, buffer, write_size);
    
    // Update the file size if we wrote past the old End-Of-File
    if (offset + write_size > this->size) {
        this->size = offset + write_size;
    }
    
    return write_size;
}

// ==========================================
// TmpFSDirectory Implementation
// ==========================================

TmpFSDirectory::TmpFSDirectory(const char* dirname) {
    // Safely copy the directory name
    size_t i = 0;
    while (dirname[i] != '\0' && i < 255) {
        this->name[i] = dirname[i];
        i++;
    }
    this->name[i] = '\0';
    
    this->flags = VFS_DIRECTORY;
    this->size = 0; // For a directory, size represents the number of items
    
    // Initialize the children array
    this->capacity = 4; // Start with space for 4 items
    this->child_count = 0;
    this->children = new VFSNode*[this->capacity];
}

TmpFSDirectory::~TmpFSDirectory() {
    // Early return if children array is already null
    if (this->children == nullptr) return;

    delete[] this->children;
}

VFSNode* TmpFSDirectory::finddir(const char* search_name) {
    // Early return for invalid input
    if (search_name == nullptr) return nullptr;

    // Iterate through all children to find a matching name
    for (int i = 0; i < this->child_count; i++) {
        // Usa il tuo strcmp!
        if (strcmp(this->children[i]->name, search_name) == 0) {
            return this->children[i]; // Found it!
        }
    }
    return nullptr; // File or directory not found
}

bool TmpFSDirectory::add_child(VFSNode* child) {
    // Early return: invalid child pointer
    if (child == nullptr) return false;

    // Early return: naming collision (file/folder already exists)
    if (this->finddir(child->name) != nullptr) return false; 

    // If the array is full, we need to resize it
    if (this->child_count >= this->capacity) {
        int new_capacity = this->capacity * 2;
        VFSNode** new_children = new VFSNode*[new_capacity];
        
        // Early return: Kernel Out of Memory
        if (new_children == nullptr) return false; 

        // Copy old pointers to the new array
        for (int i = 0; i < this->child_count; i++) {
            new_children[i] = this->children[i];
        }
        
        delete[] this->children;
        this->children = new_children;
        this->capacity = new_capacity;
    }

    // Add the new child
    this->children[this->child_count] = child;
    this->child_count++;
    this->size = this->child_count; // Update size to reflect item count
    
    return true;
}

bool TmpFSDirectory::remove_child(const char* target_name) {
    // Early return for invalid input
    if (target_name == nullptr) return false;

    for (int i = 0; i < this->child_count; i++) {
        // Usa il tuo strcmp!
        if (strcmp(this->children[i]->name, target_name) == 0) {
            // Target found: shift all subsequent elements left to close the gap
            for (int j = i; j < this->child_count - 1; j++) {
                this->children[j] = this->children[j + 1];
            }
            
            this->child_count--;
            this->size = this->child_count;
            return true;
        }
    }
    return false; // Child not found
}

VFSNode* TmpFSDirectory::readdir(uint32_t index) {
    // Early return: if the index is out of bounds, there are no more children
    if (index >= this->child_count) return nullptr;

    // Return the child at the requested index
    return this->children[index];
}