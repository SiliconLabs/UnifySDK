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
 * @defgroup zpc_dotdot_mqtt_group_dispatch ZPC Group dispatch
 * @ingroup zpc_dotdot_mqtt
 * @brief ZPC dispatching component, that will distribute an incoming command
 * to all members of a given group.
 *
 * @{
 */

#ifndef ZPC_DOTDOT_MQTT_GROUP_DISPATCH_H
#define ZPC_DOTDOT_MQTT_GROUP_DISPATCH_H

#include "uic_mqtt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief ZPC callback functions that will dispatch incoming group commands
* to individual UNID-endpoints that are part of the group.
* @param group_id       Group ID of the group to dispatch.
* @param cluster_name   The name of the Cluster for which the command was received
* @param command        The Command that was received within the Cluster.
* @param message        The MQTT message/payload received for the command
* @param message_length The length of the MQTT message/payload received for the
                        Command.
* @param callback MQTT subscription callback that is normally used for individual
                  Command dispatch for the cluster.
*/
void zpc_dotdot_mqtt_group_dispatch(uint16_t group_id,
                                    const char *cluster_name,
                                    const char *command,
                                    const char *message,
                                    size_t message_length,
                                    mqtt_message_callback_t callback);

/**
 * @brief Initializes the ZPC Group dispatch resources.
 */
sl_status_t zpc_dotdot_mqtt_group_dispatch_init();

#ifdef __cplusplus
}
#endif

#endif  //ZPC_DOTDOT_MQTT_GROUP_DISPATCH_H
/** @} end zpc_dotdot_mqtt_group_dispatch */
