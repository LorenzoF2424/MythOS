#include "fs/exfat/exfat_vfs.h"
#include "drivers/storage/ata_pio.h"
#include "lib/string.h"

// Provided by exfat.cpp
extern uint32_t exfat_cluster_to_lba(uint32_t cluster);

// ==========================================
// Name Conversion Helpers
// ==========================================

// ExFAT stores filenames as UTF-16LE.
// For a kernel with ASCII-only CLI, we convert by stripping the high byte.
// Non-ASCII characters (high byte != 0) are replaced with '?'.
void exfat_utf16_to_ascii(const uint16_t* utf16, int name_length, char* out_ascii) {
    for (int i = 0; i < name_length; i++) {
        uint16_t codepoint = utf16[i];
        out_ascii[i] = (codepoint <= 0x7F) ? (char)codepoint : '?';
    }
    out_ascii[name_length] = '\0';
}

// Converts an ASCII string to UTF-16LE by zero-extending each byte.
void exfat_ascii_to_utf16(const char* ascii, uint16_t* out_utf16, int max_chars) {
    int i = 0;
    while (ascii[i] != '\0' && i < max_chars) {
        // Uppercase everything: ExFAT name matching is case-insensitive
        char c = ascii[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        out_utf16[i] = (uint16_t)c;
        i++;
    }
    // Zero-pad the remaining slots
    while (i < max_chars) {
        out_utf16[i++] = 0x0000;
    }
}

// ExFAT name hash: iterate over each UTF-16 character uppercased,
// rotating the accumulator right by 1 bit before adding each character.
// Both the high and low bytes of each codepoint are hashed separately.
// This matches the Microsoft ExFAT specification exactly.
uint16_t exfat_name_hash(const uint16_t* name, uint8_t name_length) {
    uint16_t hash = 0;
    for (int i = 0; i < name_length; i++) {
        uint16_t c = name[i];
        // Uppercase the character (ASCII range only — safe for our driver)
        if (c >= 'a' && c <= 'z') c -= 32;

        // Hash low byte
        hash = ((hash << 15) | (hash >> 1)) + (uint8_t)(c & 0xFF);
        // Hash high byte
        hash = ((hash << 15) | (hash >> 1)) + (uint8_t)(c >> 8);
    }
    return hash;
}

// ==========================================
// ExFATFile Implementation
// ==========================================

ExFATFile::ExFATFile(const char* file_name, uint32_t cluster, uint64_t file_size) {
    strcpy(this->name, file_name);
    this->flags         = VFS_FILE;
    this->size          = file_size;
    this->start_cluster = cluster;
}

uint64_t ExFATFile::read(uint64_t offset, uint64_t read_size, uint8_t* buffer) {
    // Early return: nothing to read
    if (read_size == 0 || this->size == 0) return 0;

    // For this implementation, offset is not yet supported: we always read from
    // the start of the file. A full implementation would skip clusters accordingly.
    exfat_read_file(this->start_cluster, this->size, buffer);
    return this->size;
}

uint64_t ExFATFile::write(uint64_t offset, uint64_t write_size, const uint8_t* buffer) {
    // Early return: guard against invalid parameters
    if (buffer == nullptr || write_size == 0) return 0;

    // Early return: prevent multi-cluster writes for now (mirrors FAT32 driver)
    if (write_size > 512) {
        kprintf("Error: ExFAT writes larger than 1 sector not yet supported.\n");
        return 0;
    }

    // 1. Allocate a cluster if this file has never been written before
    if (this->start_cluster == 0) {
        this->start_cluster = exfat_find_free_cluster();

        if (this->start_cluster == 0) {
            kprintf("Error: Disk is full.\n");
            return 0;
        }

        // Mark the cluster as End-Of-Chain in the FAT (0x0FFFFFFF)
        exfat_write_fat_entry(this->start_cluster, 0x0FFFFFFF);
    }

    // 2. Zero-pad a full sector buffer to avoid writing stale RAM to disk
    uint8_t sector_buffer[512];
    memset(sector_buffer, 0, 512);
    memcpy(sector_buffer, buffer, (uint32_t)write_size);

    // 3. Write the data to the physical disk
    uint32_t data_lba = exfat_cluster_to_lba(this->start_cluster);
    ata_pio_write_sector(data_lba, sector_buffer);

    // 4. Update the in-memory size
    if (offset + write_size > this->size) {
        this->size = offset + write_size;
    }

    // 5. Persist the metadata change to the directory entry on disk
    exfat_update_directory_entry(this->name, this->start_cluster, this->size);

    return write_size;
}

VFSNode* ExFATFile::finddir(const char* name) { return nullptr; }
VFSNode* ExFATFile::readdir(uint32_t index)   { return nullptr; }

// ==========================================
// ExFATDirectory Implementation
// ==========================================

ExFATDirectory::ExFATDirectory(const char* dir_name, uint32_t cluster) {
    strcpy(this->name, dir_name);
    this->flags         = VFS_DIRECTORY;
    this->size          = 0;
    this->start_cluster = cluster;
    this->child_count   = 0;
    this->loaded        = false;

    for (int i = 0; i < MAX_CHILDREN; i++) children[i] = nullptr;
}

ExFATDirectory::~ExFATDirectory() {
    for (int i = 0; i < child_count; i++) {
        delete children[i];
    }
}

void ExFATDirectory::load_children() {
    // Early return: already loaded, use the cache
    if (loaded) return;

    uint8_t buffer[512];
    uint32_t dir_lba = exfat_cluster_to_lba(this->start_cluster);
    ata_pio_read_sector(dir_lba, buffer);

    ExFAT_GenericEntry* entries = (ExFAT_GenericEntry*)buffer;
    int total = 512 / 32; // 16 generic slots per sector

    for (int i = 0; i < total && child_count < MAX_CHILDREN; ) {
        // Early break: End Of Directory marker
        if (entries[i].entry_type == EXFAT_ENTRY_EOD) break;

        // Early continue: skip non-file primaries (Bitmap, UpCase, Label, deleted)
        if (entries[i].entry_type != EXFAT_ENTRY_FILE) {
            i++;
            continue;
        }

        ExFAT_FileEntry*   file   = (ExFAT_FileEntry*)  &entries[i];
        ExFAT_StreamEntry* stream = (ExFAT_StreamEntry*) &entries[i + 1];
        ExFAT_NameEntry*   name   = (ExFAT_NameEntry*)   &entries[i + 2];

        // Sanity: we need at least the stream + name secondary entries
        if (file->secondary_count < 2 || i + 2 >= total) {
            i += 1 + file->secondary_count;
            continue;
        }

        // Decode the UTF-16LE filename to ASCII
        char ascii_name[256];
        exfat_utf16_to_ascii(name->name, stream->name_length, ascii_name);

        uint32_t cluster = stream->first_cluster;
        uint64_t size    = stream->data_length;
        bool     is_dir  = (file->file_attributes & EXFAT_ATTR_DIRECTORY);

        if (is_dir) {
            children[child_count] = new ExFATDirectory(ascii_name, cluster);
        } else {
            children[child_count] = new ExFATFile(ascii_name, cluster, size);
        }
        child_count++;

        // Advance past the whole entry set
        i += 1 + file->secondary_count;
    }

    loaded = true;
}

VFSNode* ExFATDirectory::readdir(uint32_t index) {
    load_children();

    // Early return: index out of bounds
    if ((int)index >= child_count) return nullptr;

    return children[index];
}

VFSNode* ExFATDirectory::finddir(const char* target_name) {
    load_children();

    for (int i = 0; i < child_count; i++) {
        if (strcmp(children[i]->name, target_name) == 0) {
            return children[i];
        }
    }
    return nullptr;
}

uint64_t ExFATDirectory::read (uint64_t, uint64_t, uint8_t*)        { return 0; }
uint64_t ExFATDirectory::write(uint64_t, uint64_t, const uint8_t*) { return 0; }

bool ExFATDirectory::create_file(const char* new_filename) {
    uint8_t buffer[512];
    uint32_t dir_lba = exfat_cluster_to_lba(this->start_cluster);
    ata_pio_read_sector(dir_lba, buffer);

    ExFAT_GenericEntry* entries = (ExFAT_GenericEntry*)buffer;
    int total = 512 / 32;

    // An ExFAT file needs 3 contiguous free slots: File + Stream + FileName
    int free_slot = -1;
    for (int i = 0; i <= total - 3; i++) {
        if (entries[i].entry_type == EXFAT_ENTRY_EOD ||
            entries[i].entry_type == EXFAT_ENTRY_DELETED) {
            // Check the next two slots are also free
            if ((entries[i+1].entry_type == EXFAT_ENTRY_EOD ||
                 entries[i+1].entry_type == EXFAT_ENTRY_DELETED) &&
                (entries[i+2].entry_type == EXFAT_ENTRY_EOD ||
                 entries[i+2].entry_type == EXFAT_ENTRY_DELETED)) {
                free_slot = i;
                break;
            }
        }
    }

    // Early return: no room in this sector
    if (free_slot == -1) {
        kprintf("Error: ExFAT directory sector is full. Cannot create file.\n");
        return false;
    }

    // ── Build the File Entry (primary) ──────────────────────────────────────
    ExFAT_FileEntry* file = (ExFAT_FileEntry*)&entries[free_slot];
    memset(file, 0, sizeof(ExFAT_FileEntry));
    file->entry_type      = EXFAT_ENTRY_FILE;
    file->secondary_count = 2;             // Stream Extension + File Name
    file->set_checksum    = 0;             // Simplified: a real driver would compute this
    file->file_attributes = EXFAT_ATTR_ARCHIVE;

    // ── Build the Stream Extension (first secondary) ─────────────────────────
    ExFAT_StreamEntry* stream = (ExFAT_StreamEntry*)&entries[free_slot + 1];
    memset(stream, 0, sizeof(ExFAT_StreamEntry));
    stream->entry_type         = EXFAT_ENTRY_STREAM;
    stream->general_flags      = 0x01;    // AllocationPossible = 1
    stream->first_cluster      = 0;       // No data allocated yet
    stream->data_length        = 0;
    stream->valid_data_length  = 0;

    // Compute name length and hash
    int name_len = 0;
    while (new_filename[name_len] != '\0') name_len++;
    stream->name_length = (uint8_t)name_len;

    // ── Build the File Name Entry (second secondary) ─────────────────────────
    ExFAT_NameEntry* name_entry = (ExFAT_NameEntry*)&entries[free_slot + 2];
    memset(name_entry, 0, sizeof(ExFAT_NameEntry));
    name_entry->entry_type    = EXFAT_ENTRY_NAME;
    name_entry->general_flags = 0x00;
    exfat_ascii_to_utf16(new_filename, name_entry->name, 15);

    // Now that we have the UTF-16 name, compute and store its hash
    stream->name_hash = exfat_name_hash(name_entry->name, stream->name_length);

    // ── Flush to disk ────────────────────────────────────────────────────────
    ata_pio_write_sector(dir_lba, buffer);

    // ── Update the in-memory VFS tree ───────────────────────────────────────
    if (child_count < MAX_CHILDREN) {
        children[child_count] = new ExFATFile(new_filename, 0, 0);
        child_count++;
    }

    return true;
}