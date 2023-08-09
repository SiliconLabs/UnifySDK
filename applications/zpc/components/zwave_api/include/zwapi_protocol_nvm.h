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
 * @file zwapi_protocol_nvm.h
 * @brief Z-Wave API for NVM manipulation.
 */

#ifndef ZWAPI_PROTOCOL_NVM_H
#define ZWAPI_PROTOCOL_NVM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ZWAPI_NVM NVM API
 * @ingroup zwave_api
 * @brief Functions to access the NVM in a Z-Wave module.
 *
 * Functions in this module are used to access Non-Volatile memory in aZ-Wave
 * module.
 * @{
 */

/// @name NVM operations
///@{
#define ZWAVE_API_NVM_OPEN_OPERATION  0x00
#define ZWAVE_API_NVM_READ_OPERATION  0x01
#define ZWAVE_API_NVM_WRITE_OPERATION 0x02
#define ZWAVE_API_NVM_CLOSE_OPERATION 0x03
///@}

/// @name NVM return codes from Z-Wave module
///@{
#define ZWAVE_API_NVM_RETURN_CODE_OK                        0x00
#define ZWAVE_API_NVM_RETURN_CODE_ERROR                     0x01
#define ZWAVE_API_NVM_RETURN_CODE_ERROR_OPERATION_MISMATCH  0x02
#define ZWAVE_API_NVM_RETURN_CODE_ERROR_OPERATION_DISTURBED 0x03
#define ZWAVE_API_NVM_RETURN_CODE_END_OF_FILE               0xFF
///@}

/// NVM read and write status codes
typedef enum {
  ZWAPI_PROTOCOL_NVM_STATUS_OK    = 0,  ///< Read/write operation successful
  ZWAPI_PROTOCOL_NVM_STATUS_ERROR = 1,  ///< Unspecified error occurred
  ZWAPI_PROTOCOL_NVM_STATUS_EOF
  = 2,  ///< Read/write operation successful and has reached the end of file.
} zwapi_protocol_nvm_status_t;

/**
 * @brief Close the NVM (restart the radio) after a read or write operation.
 *
 * @returns Status code of  the close operation:
 *  - @ref ZWAVE_API_NVM_RETURN_CODE_OK Success
 *  - @ref ZWAVE_API_NVM_RETURN_CODE_ERROR Generic error
 *  - @ref ZWAVE_API_NVM_RETURN_CODE_ERROR_OPERATION_MISMATCH Error: Read/Write
 *    operation mismatch
 *  - @ref ZWAVE_API_NVM_RETURN_CODE_ERROR_OPERATION_DISTURBED Error: Read
 *    operation was disturbed by another write
 *  - @ref ZWAVE_API_NVM_RETURN_CODE_END_OF_FILE End of File
 *
 * aka SerialAPI_nvm_close
 */
uint8_t zwapi_nvm_close(void);

/**
 * @brief Open the NVM (stops the radio) before read or write operation.
 *
 * @returns uint32_t the expected size in bytes of the NVM area.
 *
 * aka SerialAPI_nvm_open
 */
uint32_t zwapi_nvm_open(void);

/**
 * @brief Read a chunk of the 500-series NVM or 700-series NVM3.
 *
 * @param offset NVM offset (in bytes) from which to start reading data
 * @param buf Reading buffer where the read data will be returned
 * @param length the desired amount of bytes to read from "offset"
 * @param length_read pointer that reports the actual amount of bytes read
 * @returns Status Code for the read operation:
 * - @ref ZWAPI_PROTOCOL_NVM_STATUS_OK Status OK
 * - @ref ZWAPI_PROTOCOL_NVM_STATUS_ERROR Unspecified Error
 * - @ref ZWAPI_PROTOCOL_NVM_STATUS_EOF Status End-of-file
 *
 * aka SerialAPI_nvm_backup
 */
zwapi_protocol_nvm_status_t zwapi_nvm_read(uint32_t offset,
                                           uint8_t *buf,
                                           uint8_t length,
                                           uint8_t *length_read);

/**
 * @brief Write a chunk of the 500-series NVM or 700-series NVM3.
 *
 * @param offset NVM offset (in bytes) from which to start writing data
 * @param buf buffer containing the data to be written to the NVM
 * @param length the desired amount of bytes to written from "offset"
 * @param length_written pointer that reports the actual amount of bytes written
 * @returns Status Code for the write operation:
 * - @ref ZWAPI_PROTOCOL_NVM_STATUS_OK Status OK
 * - @ref ZWAPI_PROTOCOL_NVM_STATUS_ERROR Unspecified Error
 * - @ref ZWAPI_PROTOCOL_NVM_STATUS_EOF Status End-of-file
 *
 * aka SerialAPI_nvm_restore
 */
zwapi_protocol_nvm_status_t zwapi_nvm_write(uint32_t offset,
                                            const uint8_t *buf,
                                            uint8_t length,
                                            uint8_t *length_written);

/// @} End of defgroup ZWAPI_NVM

#ifdef __cplusplus
}
#endif

#endif  // ZWAPI_PROTOCOL_NVM_H
