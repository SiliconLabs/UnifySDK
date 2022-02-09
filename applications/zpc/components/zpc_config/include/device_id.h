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
 * @file device_id.h
 * @brief API to get device ID
 */

#ifndef DEVICE_ID_H
#define DEVICE_ID_H
/**
 * @brief Get unique reboot persistent device id
 *
 * Can be used in identifying the device, ZPC is running on
 * for e.g. in Device specific report Command class
 *
 */
const char* get_device_id();

#endif  // ZWAPI_SERIAL_H
