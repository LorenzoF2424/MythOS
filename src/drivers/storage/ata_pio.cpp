#include "drivers/storage/ata_pio.h"
#include "kernel/idt/idt.h" // Update this if your inb/outb functions are elsewhere

// ==========================================
// ATA Primary Bus I/O Ports
// ==========================================
#define ATA_PORT_DATA       0x1F0
#define ATA_PORT_ERROR      0x1F1
#define ATA_PORT_SECT_COUNT 0x1F2
#define ATA_PORT_LBA_LO     0x1F3
#define ATA_PORT_LBA_MID    0x1F4
#define ATA_PORT_LBA_HI     0x1F5
#define ATA_PORT_DRV        0x1F6
#define ATA_PORT_COMMAND    0x1F7
#define ATA_PORT_STATUS     0x1F7

// ==========================================
// ATA Commands & Status Flags
// ==========================================
#define ATA_CMD_READ_PIO    0x20
#define ATA_CMD_WRITE_PIO   0x30

#define ATA_SR_BSY          0x80    // Busy (The drive is spinning/thinking)
#define ATA_SR_DRQ          0x08    // Data Request (The drive is ready to exchange data)

// ==========================================
// Helper Functions (Private)
// ==========================================

// Waits until the drive is no longer busy
static void ata_wait_bsy() {
    while (inb(ATA_PORT_STATUS) & ATA_SR_BSY);
}

// Waits until the drive is ready to transfer data
static void ata_wait_drq() {
    while (!(inb(ATA_PORT_STATUS) & ATA_SR_DRQ));
}

// ==========================================
// Driver Implementation
// ==========================================

void ata_pio_read_sector(uint32_t lba, uint8_t* buffer) {
    ata_wait_bsy();

    // 1. Select the Drive (Master) and set LBA mode with the highest 4 bits
    // 0xE0 means "LBA mode" and "Master drive"
    outb(ATA_PORT_DRV, 0xF0 | ((lba >> 24) & 0x0F));    
    // 2. Set the number of sectors to read (1)
    outb(ATA_PORT_SECT_COUNT, 1);
    
    // 3. Send the lower 24 bits of the LBA address split across 3 ports
    outb(ATA_PORT_LBA_LO, (uint8_t)lba);
    outb(ATA_PORT_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PORT_LBA_HI, (uint8_t)(lba >> 16));
    
    // 4. Send the READ command
    outb(ATA_PORT_COMMAND, ATA_CMD_READ_PIO);

    // 5. Wait for the drive to physically find the data and be ready to send it
    ata_wait_bsy();
    ata_wait_drq();

    // 6. Read 512 bytes. Since we read 16 bits (2 bytes) at a time, we do 256 loops.
    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = inw(ATA_PORT_DATA);
    }
}

void ata_pio_write_sector(uint32_t lba, const uint8_t* buffer) {
    ata_wait_bsy();

    // Steps 1-3 are identical to reading: select the destination
    outb(ATA_PORT_DRV, 0xF0 | ((lba >> 24) & 0x0F));
    outb(ATA_PORT_SECT_COUNT, 1);
    outb(ATA_PORT_LBA_LO, (uint8_t)lba);
    outb(ATA_PORT_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PORT_LBA_HI, (uint8_t)(lba >> 16));
    
    // 4. Send the WRITE command
    outb(ATA_PORT_COMMAND, ATA_CMD_WRITE_PIO);

    // 5. Wait for the drive to be ready to receive our data
    ata_wait_bsy();
    ata_wait_drq();

    // 6. Write 512 bytes (2 bytes at a time)
    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(ATA_PORT_DATA, ptr[i]);
    }
    
    // Optional but recommended: flush the drive cache (ATA_CMD_CACHE_FLUSH = 0xE7)
    // to ensure data is written to the magnetic platters.
    // outb(ATA_PORT_COMMAND, 0xE7);
    // ata_wait_bsy();
}

