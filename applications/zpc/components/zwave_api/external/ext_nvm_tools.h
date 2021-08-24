// CLEANUP: Move out of zwave_api (e.g. to zpc_utils)

#ifndef NVM_TOOLS_H 
#define NVM_TOOLS_H 

#include <stdbool.h>
#include <stdint.h>

enum nvm_backup_restore_operation {
   nvm_backup_restore_open = 0x00,
   nvm_backup_restore_read = 0x01,
   nvm_backup_restore_write = 0x02,
   nvm_backup_restore_close = 0x03,
};

/* How many bytes we pass at a time when writing in the NVM via the SerialAPI */
#define NVM_WRITE_CHUNK_SIZE			64
/**
 * Read the NVM from a device and write it into an output file
 *
 * \param filename where the NVM backup should be stored
 * \param chiptype Type of chip to write to ZW_GECKO_CHIP_TYPE (700 serries) or ZW_CHIP_TYPE (500 serries)
 */
bool ZW_NVM_Backup(const char* filename,uint8_t chiptype);
/**
 * Write the NVM from an input file to a 500 or 700 series device
 *
 * \param filename from where the NVM backup should be restored
 * \param chiptype Type of chip to write to ZW_GECKO_CHIP_TYPE (700 serries) or ZW_CHIP_TYPE (500 serries)
 */
bool ZW_NVM_Restore(const char* filename,uint8_t chiptype);

#endif
