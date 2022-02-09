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
 * @file zwave_api.h
 * @brief Main include file for the Z-Wave API (only need to include this one)
 */

#ifndef ZWAVE_API_H
#define ZWAVE_API_H

/**
 * @defgroup zwave_api Z-Wave API
 * @ingroup zwave_controller
 * @brief Access to the serial API of a Z-Wave module
 *
 * The Z-Wave API is a set of interface functions and supporting datatypes used
 * to access the serial API of an attached Z-Wave module.
 *
 * Functions that do not complete the requested action before returning to the
 * application (e.g., zwapi_send_data_bridge()) have a callback function pointer
 * as one of the entry parameters. Unless explicitly specified, this function
 * pointer can be set to NULL (no action to take on completion).
 *
 * \image html zwave_api_overview.png "Z-Wave API overview"
 *
 */

#include "zwapi_init.h"
#include "zwapi_protocol_basis.h"
#include "zwapi_protocol_controller.h"
#include "zwapi_protocol_mem.h"
#include "zwapi_protocol_nvm.h"
#include "zwapi_protocol_transport.h"

/**
* @page zgw_namemap Legacy Z/IP Gateway Serial-API Names
* @ingroup zwave_api
* @brief Mapping between the names used in the Z/IP codebase versus the ZPC Z-Wave API
*       names.
*/
#endif  // ZWAVE_API_H
