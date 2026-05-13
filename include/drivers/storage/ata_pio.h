#ifndef DRIVERS_STORAGE_ATA_PIO_H
#define DRIVERS_STORAGE_ATA_PIO_H

#include <stdint.h>

// ==========================================
// ATA PIO Storage Driver API
// ==========================================

// Reads a single sector (512 bytes) from the primary Master drive
void ata_pio_read_sector(uint32_t lba, uint8_t* buffer);

// Writes a single sector (512 bytes) to the primary Master drive
void ata_pio_write_sector(uint32_t lba, const uint8_t* buffer);

#endif // DRIVERS_STORAGE_ATA_PIO_H