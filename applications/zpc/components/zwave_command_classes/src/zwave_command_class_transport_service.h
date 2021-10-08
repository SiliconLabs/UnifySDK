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
 * @defgroup zwave_command_class_transport_service Transport Service Command Class
 * @ingroup command_classes
 * @brief Transport Service Command Class support and control handler
 *
 * This Command Class handler is an empty shell, indicating that we do support
 * and control the Transport Service Command Class. It does not support any
 * additional commands.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_TRANSPORT_SERVICE_H
#define ZWAVE_COMMAND_CLASS_TRANSPORT_SERVICE_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

sl_status_t zwave_command_class_transport_service_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_TRANSPORT_SERVICE_H
/** @} end zwave_command_class_transport_service */
