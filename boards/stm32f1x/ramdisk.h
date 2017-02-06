
#ifndef __RAMDISK_H
#define __RAMDISK_H

#include <stdint.h>

extern int ramdisk_init(void);
extern int ramdisk_read(uint32_t lba, uint8_t *copy_to);
extern int ramdisk_write(uint32_t lba, const uint8_t *copy_from);
extern int ramdisk_blocks(void);

extern int sysdisk_init(void);
extern int sysdisk_read(uint32_t lba, uint8_t *copy_to);
extern int sysdisk_write(uint32_t lba, const uint8_t *copy_from);
extern int sysdisk_blocks(void);

#endif
