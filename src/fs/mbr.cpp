#include "fs/mbr.h"
#include "drivers/storage/ata_pio.h"
// Ensure you include your kprintf header
// #include "cli/CLI.h" 

void mbr_read_partitions() {
    uint8_t buffer[512];
    
    kprintf("Reading Sector 0 (MBR)...\n");
    
    // Ask the hardware to read the first sector of the Master drive
    ata_pio_read_sector(0, buffer);

    // Map the raw bytes onto our structure
    MBR* mbr = (MBR*)buffer;

    // Early return: check if it's a valid formatted disk
    if (mbr->magic_signature != 0xAA55) {
        kprintf("Error: Invalid MBR signature. No readable disk found.\n");
        return;
    }

    kprintf("MBR Found! Scanning Partition Table:\n");

    // Read the 4 partition slots
    for (int i = 0; i < 4; i++) {
        PartitionEntry* part = &mbr->partitions[i];

        // Early continue if the partition slot is empty
        if (part->type == 0x00) continue;

        // Calculate size in Megabytes (sectors * 512 bytes / 1024 / 1024)
        uint32_t size_mb = (part->sector_count * 512) / (1024 * 1024);

        kprintf(" Partition %d -> Type: %02X | Start (LBA): %d | Size: %d MB\n", 
                i + 1, part->type, part->lba_start, size_mb);

        // Identify if it's a partition we will be able to read (FAT32)
        switch (part->type) {

            case 0x0B: // FAT32 (CHS)
            case 0x0C: // FAT32 (LBA)
                kprintf("  * FAT32 partition detected.\n");
                fat32_init(part->lba_start);
            break;

            case 0x07: // Installable File System (NTFS or exFAT)
                kprintf("  * Potential exFAT partition detected.\n");
                exfat_init(part->lba_start);
            break;

            default:
                kprintf("  * Unknown partition type %02X. Skipping...\n", part->type);
            break;
        }
    }
}