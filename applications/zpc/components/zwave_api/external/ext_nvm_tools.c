// CLEANUP: Move out of zwave_api (e.g. to zpc_utils)

#include <unistd.h>
#include <stdio.h>
#include "Serialapi.h"
#include <errno.h>
#include "nvm_tools.h"
static int set_default_cb_called = 0;

static void Set_Default_Callback() {
  set_default_cb_called = 1;
}

bool ZW_NVM_Backup(const char* filename,uint8_t chiptype)
{
  uint32_t backup_length = 0;
  uint8_t length_read = 0;
  uint16_t offset = 0;
  uint8_t read_status = 0;
  uint8_t nvm_close_status = 0;
  uint8_t read_buffer[ REQUEST_BUFFER_SIZE ];
  FILE *my_nvm_backup_file;

  /* Open up a file */
  my_nvm_backup_file = fopen(filename, "wb");
  if (NULL == my_nvm_backup_file)
  {
    printf("Cannot create NVM file \"%s\": %s.\n", filename, strerror(errno));
    return false;
  }

  /* According to INS12350-17:
  The correct sequence of commands for initiating a backup is the following:
  FUNC_ID_NVM_BACKUP_RESTORE (open) Returns the backup size.
  FUNC_ID_NVM_BACKUP_RESTORE (read, read, .)Returns EOF if no more data or error if the backup is disturbed by other writes to the NVM.
  FUNC_ID_NVM_BACKUP_RESTORE (close) Returns an error if backup was disturbed by other writes. Ok is returned if the backup was done without any writes to the NVM.
  If an error was returned, discard backed up data and try again.
  */
  printf("Starting NVM read operation...\n");
  backup_length = zwapi_nvm_open();
  printf("backup_length: %d\n", backup_length);

  // Read the NVM chunck by chunk and put it into the file
  while (offset<backup_length && 0 == read_status) {
    read_status = zwapi_nvm_read(offset, read_buffer, REQUEST_BUFFER_SIZE, &length_read);
    offset+=length_read;
    fwrite(read_buffer, sizeof(uint8_t), length_read, my_nvm_backup_file);

    /* Print the progress*/
    fflush(stdout);
    printf("\rReading %d / %d bytes...",offset,backup_length-1);
  }
  printf("\n");

  // Close the local file.
  fclose(my_nvm_backup_file);

  if (read_status == 2)
  {
    printf("NVM read successfully\n");
  }
  else
  {
    printf("ERROR: Reading NVM failed. Read status %d returned\n",read_status);
    return false;
  }

  nvm_close_status = zwapi_nvm_close();
  if (nvm_close_status != 0)
  {
    printf("ERROR: NVM close command failed.\n");
    return false;
  }

 /* On 700-series chips, zwapi_nvm_close must be followed by a chip reset */
  if (chiptype == ZW_GECKO_CHIP_TYPE) {
    zwapi_soft_reset();
  }

  return true;
}

bool ZW_NVM_Restore(const char* filename, uint8_t chiptype)
{
  FILE *my_nvm_backup_file;
  uint8_t length_write = 0;
  uint8_t length_written = 0;
  uint16_t offset = 0;
  uint8_t serial_api_status = 0 ;
  uint8_t write_status = 0;
  uint8_t nvm_close_status = 0;
  uint32_t expected_length = 0;
  uint8_t buffer[NVM_WRITE_CHUNK_SIZE];

  /* Open the backup file */
  my_nvm_backup_file = fopen(filename, "rb");
  if (NULL == my_nvm_backup_file)
  {
    printf("Cannot open NVM file \"%s\": %s.\n", filename, strerror(errno));
    return false;
  }

  /*
  According to INS12350-17
  The correct sequence of commands for initiating a restore is the following:
  FUNC_ID_ZW_SET_DEFAULT Deletes all old NVM content
  FUNC_ID_NVM_BACKUP_RESTORE (open) Returns an unused value
  FUNC_ID_NVM_BACKUP_RESTORE (write, write, ....) Writes the whole NVM image to the NVM in the Z-Wave module
  FUNC_ID_NVM_BACKUP_RESTORE (close) RReturns an unused value.
  FUNC_ID_SERIAL_API_SOFT_RESET Activates the Z-Wave module with the new NVM image.
  If an error was returned, discard backed up data and try again.
  */

  // Set the module to default:
  printf("Reset the module to default...\n");
  set_default_cb_called = 0;

  zwapi_set_default(Set_Default_Callback);
  //This is a serial api hack to wait for the callback
  for(int i=0; i <10000; i++){
    zwapi_poll();
    if((set_default_cb_called) ) break;
  }
  if(!set_default_cb_called) {
    return false;
  }

  printf("Starting NVM Write operation...\n");
  expected_length = zwapi_nvm_open();

  /* Read in the file and push it write it on to the NVM  */
  length_write = fread(buffer, sizeof(uint8_t), NVM_WRITE_CHUNK_SIZE, my_nvm_backup_file);
  while (0 != length_write)
  {
    write_status = zwapi_nvm_write(offset, buffer, length_write, &length_written);
    offset += length_written;
    if (length_written != length_write)
    {
      printf("ERROR : Something went wrong when writing NVM. Aborting\n");
      break;
    }
    /* Look in the file to see if there is more to write */
    length_write = fread(buffer, sizeof(uint8_t), NVM_WRITE_CHUNK_SIZE, my_nvm_backup_file);

    if(0 > length_write && 0 != write_status)
    {
      printf("ERROR : More data to write from file, but could not write on NVM (write status = %d, length write = %d\n", write_status,length_write);
      break;
    }

    /* Print the progress */
    fflush(stdout);
    printf("\rWritten %d / %d bytes...",offset,expected_length);
  }
  printf("\n");

  /* We can now close the local file */
  fclose(my_nvm_backup_file);

  nvm_close_status = zwapi_nvm_close();
  if (0 != nvm_close_status)
  {
    printf("ERROR: NVM close command failed.\n");
    return false;
  }

  // Perform a soft reset on the module
  printf("Soft reset of the module\n");

 /*The WatchDog is used to reset the chip after restoring EEPROM image to Z-Wave module from backup on 500 series,
  *in 700 series: the FUNC_ID_ZW_WATCHDOG_ENABLE will be removed, therefore, zwapi_soft_reset() should be used*/
  if (chiptype == ZW_GECKO_CHIP_TYPE) {
    zwapi_soft_reset();
  }
  else {
    zwapi_enable_watchdog();
  }

  return true;
}


