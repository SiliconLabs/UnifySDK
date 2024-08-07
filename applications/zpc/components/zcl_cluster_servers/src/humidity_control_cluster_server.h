/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef HUMIDITY_CONTROL_CLUSTER_SERVER_H
#define HUMIDITY_CONTROL_CLUSTER_SERVER_H

// Generic includes
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Humidity Control cluster server
 *
 * @returns true on success
 * @returns false on failure
 *
 */
sl_status_t humidity_control_cluster_server_init(void);

#ifdef __cplusplus
}
#endif

#endif  //HUMIDITY_CONTROL_CLUSTER_SERVER_H
/** @} end humidity_control_cluster_server */
