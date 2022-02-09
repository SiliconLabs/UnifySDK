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
 * @defgroup zwave_s2_transport Security 2 Transport
 * @ingroup zwave_transports
 * @brief Transport for Security 2
 *
 * @{
 */

#ifndef ZWAVE_S2_TRANSPORT_H
#define ZWAVE_S2_TRANSPORT_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set the secure NIF.
 *
 * Set the secure NIF which is sent by when someone does a secure commands
 * supported get.
 *
 * @param command_class_list         List of command classes which are supported
 *                                   securely by the controller.
 * @param command_class_list_length  Length of command class
 * @return sl_status_t
 * - SL_STATUS_OK                   on success
 * - SL_STATUS_WOULD_OVERFLOW       if list is too long.
 */
sl_status_t zwave_s2_set_secure_nif(const uint8_t *command_class_list,
                                    uint8_t command_class_list_length);

/**
 * @brief Initializes the Z-Wave S2 Transport
 */
void zwave_s2_transport_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_S2_TRANSPORT_H
/** @} end zwave_s2_transport */
