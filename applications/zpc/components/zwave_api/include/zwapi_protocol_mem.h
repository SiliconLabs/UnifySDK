/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @file zwapi_protocol_mem.h
 * @brief Z-Wave API for module memory manipulation
 */

#ifndef ZWAPI_PROTOCOL_MEM_H
#define ZWAPI_PROTOCOL_MEM_H

#include <stdint.h>

// Includes from other components
#include "sl_status.h"
#include "zwave_node_id_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ZWAPI_MEM Memory API
 * @ingroup zwave_api
 * @brief Functions to access storage in a Z-Wave module
 *
 * Functions in this module are used to access storage in Z-Wave module.
 * @{
 */

/* Size definitions for the NVR structure */
#define NVR_SAW_CENTER_FREQ_SIZE      0x03
#define NVR_NVM_SIZE_SIZE             0x02
#define NVR_NVM_PAGE_SIZE_SIZE        0x02
#define NVR_UUID_SIZE                 0x10
#define NVR_USBID_SIZE                0x02
#define NVR_SECURITY_PUBLIC_KEY_SIZE  0x20
#define NVR_SECURITY_PRIVATE_KEY_SIZE 0x20
#define NVR_CRC16_SIZE                0x02

/* Structure of the data in the NVR flash page */
typedef struct _NVR_FLASH_STRUCT_ {
  uint8_t bRevision;
  uint8_t bCrystalCalibration;
  uint8_t bPinSwap;
  uint8_t bNVMChipSelect;
  uint8_t abSAWCenterFreq[NVR_SAW_CENTER_FREQ_SIZE];
  uint8_t bSAWBandwidth;
  uint8_t bNVMType;
  uint8_t bNVMSize[NVR_NVM_SIZE_SIZE];
  uint8_t bNVMPageSize[NVR_NVM_PAGE_SIZE_SIZE];
  uint8_t abUUID[NVR_UUID_SIZE];
  uint8_t idVendorUsb
    [NVR_USBID_SIZE]; /*idVendor if 0xff -> use sigma Vendor ID (Assigned by USB Org)*/
  uint8_t idProductUsb
    [NVR_USBID_SIZE]; /*idProduct  if 0xff -> use sigma Product ID (Assigned by Manufacturer)*/
  uint8_t bTxCalibration1;
  uint8_t bTxCalibration2;
  uint8_t aSecurityPublicKey[NVR_SECURITY_PUBLIC_KEY_SIZE];
  uint8_t aSecurityPrivateKey[NVR_SECURITY_PRIVATE_KEY_SIZE];
} NVR_FLASH_STRUCT;

/**
 * @brief Get a value from the NVR flash page.
 *
 * @param bOffset Offset 0 is the first byte in the protocol NVR area.
 * @param bLength Number of bytes to read
 * @param bRetBuffer Pointer to buffer to receive value
 * @returns SL_STATUS_OK on success and SL_STATUS_FAIL if something went wrong
 *
 * If the CRC16 field in the protocol area of the NVR is not correct all fields
 * will return the value 0xFF when read with this function.
 *
 * @zgw_name ZW_NVRGetValue
 */
sl_status_t
  zwapi_nvr_get_value(uint8_t bOffset, uint8_t bLength, uint8_t *bRetBuffer);

/**
 * @brief Get the Home ID and Node ID of the Z-Wave module.
 *
 * @param home_id Home ID pointer (stored in host endianness)
 * @param node_id Node ID pointer
 * @returns SL_STATUS_OK on success and SL_STATUS_FAIL if something went wrong
 *
 * @zgw_name MemoryGetID
 */
sl_status_t zwapi_memory_get_ids(zwave_home_id_t *home_id,
                                 zwave_node_id_t *node_id);

/**
 * @brief Read one byte from the EEPROM.
 *
 * @param offset Application area offset
 * @param byte_value pointer for the function to store the read byte
 * @returns SL_STATUS_OK on success and SL_STATUS_FAIL if something went wrong
 *
 * @zgw_name MemoryGetByte
 */
sl_status_t zwapi_memory_get_byte(uint16_t offset, uint8_t *byte_value);

/**
 * @brief Add one byte to the EEPROM write queue.
 *
 * @param offset Application area offset
 * @param bData Data to store
 * @returns 0 on error, 1 on OK but no change, 2 Ok with changes
 *
 * @zgw_name MemoryPutByte
 */
uint8_t zwapi_memory_put_byte(uint16_t offset, uint8_t bData);

/**
 * @brief Read the number of bytes from the EEPROM.
 *
 * @param offset   Application area offset
 * @param buffer   Buffer pointer
 * @param length   Number of bytes to read
 * @returns SL_STATUS_OK on success and SL_STATUS_FAIL if something went wrong
 *
 * @zgw_name MemoryGetBuffer
 */
sl_status_t
  zwapi_memory_get_buffer(uint16_t offset, uint8_t *buffer, uint8_t length);

/**
 * @brief Copy the number of bytes to the EEPROM.
 *
 * @param offset  Application area offset
 * @param buffer  Buffer pointer. NULL pointer can be passed to zero out an area
 * @param length  Number of bytes to write
 * @param callback_function Write completed function pointer
 * @returns 0 on error, 1 on OK but no change, >=2 represents the number
 * of bytes written +1
 *
 * @zgw_name MemoryPutBuffer
 */
uint8_t zwapi_memory_put_buffer(uint16_t offset,
                                const uint8_t *buffer,
                                uint16_t length,
                                void (*callback_function)(void));

/**
 * @brief Copy the number of bytes to the EEPROM and waits for Z-Wave module callback
 * before returning.
 *
 * @param offset  Application area offset
 * @param buffer  Buffer pointer. NULL pointer can be passed to zero out an area
 * @param length  Number of bytes to write
 * @param timeout Time in ms to wait for the Z-Wave Module callback.
 * @returns 0 on error, 1 on OK but no change, >=2 represents the number
 * of bytes written +1
 */
uint8_t zwapi_memory_put_buffer_blocking(uint16_t offset,
                                         uint8_t *buffer,
                                         uint16_t length,
                                         uint16_t timeout);

/// @} End of defgroup ZWAPI_MEM

#ifdef __cplusplus
}
#endif

#endif  // ZWAPI_PROTOCOL_MEM_H
