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
 * @defgroup zpc_dotdot_mqtt ZPC Dotdot MQTT
 * @ingroup zpc_components
 * @brief ZPC Specific part of the @ref dotdot_mqtt component.
 *
 * This component is responsible for publishing the state of the ZCL attributes
 * for each unid/endpoint in the ZPC's network
 * It is also responsible for providing a function to dotdot MQTT that will
 * dispatch group commands (ZLC Commands received in the ucl/by-group/# space)
 *
 * @{
 */

#ifndef ZPC_DOTDOT_MQTT_H
#define ZPC_DOTDOT_MQTT_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the ZPC dotdot MQTT.
 *
 */
sl_status_t zpc_dotdot_mqtt_init();

#ifdef __cplusplus
}
#endif

#endif  //ZPC_DOTDOT_MQTT_H
/** @} end zpc_dotdot_mqtt */
