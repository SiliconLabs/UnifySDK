
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
 * @file zigpc_binding_setup.h
 * @defgroup zigpc_binding_setup
 * @ingroup zigpc_components
 * @brief API and functions for setting up and shutting down the
 * zigpc_bindings component
 *
 * @{
 */

#ifndef ZIGPC_BINDING_SETUP_H
#define ZIGPC_BINDING_SETUP_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief zigpc_binding_setup - set up the zigpc_binding component
 *
 * @return SL_STATUS_OK if the component could be set up properly
 */ 
sl_status_t zigpc_binding_setup();

/**
 * @brief zigpc_binding_shutdown - shutdown the zigpc_binding component
 *
 * @return 0 if the component could be shutdown properly
 */
int zigpc_binding_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif //ZIGPC_BINDING_SETUP_H
