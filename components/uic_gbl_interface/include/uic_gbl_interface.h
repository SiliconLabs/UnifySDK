/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file uic_gbl_interface.h
 * @defgroup uic_gbl_interface
 * @ingroup unify_components
 * @brief This component interfaces with the Gecko bootloader to perform OTA updates
 *
 *
 * @{
 */

#ifndef UIC_GBL_INTERFACE_H
#define UIC_GBL_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"

/**
 * @brief Callback function used to monitor the transfer progress of the file
 * 
 * @param offset Offset of last transferred block
 * @param size  Total file size.
 */
typedef void (*uic_gbl_interface_transfer_progress_t)(int offset, int size);

/**
 * @brief Probe for Gecko Bootloader
 * 
 * Detect if there is a gecko bootloader in the other end of the file descriptor
 * 
 * @param serial_fd 
 * @return sl_status_t SL_STATUS_OK on success
 */
sl_status_t uic_gbl_interface_detect_bootloader(int serial_fd);

/**
 * @brief Perfrom at DFU firmware update using the gecko bootloader
 * 
 * This function transfers an image to a device using the gecko bootloader.
 * It is assumed that the chip is already in DFU mode when this function 
 * is invoked.
 * 
 * @param gbl_filename GBL file to transfer
 * @param serial_fd File descripter to the serial interafce of the bootloader. Data will
 * be read and written to this file descriptor.
 * @param progress_callback Callback function which indicates how may bytes has
 * been transferred.
 * @return sl_status_t SL_STATUS_OK on success.
 */
sl_status_t uic_gbl_interface_transfer_image(
  const char *gbl_filename,
  int serial_fd,
  uic_gbl_interface_transfer_progress_t progress_callback);

#ifdef __cplusplus
}
#endif

#endif  //UIC_GBL_INTERFACE_H
/** @} end uic_gbl_interface */
