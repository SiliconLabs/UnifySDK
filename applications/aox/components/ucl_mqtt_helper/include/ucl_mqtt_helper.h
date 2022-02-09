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
 * @file
 * @brief UCL helper module for MQTT messaging.
 */

#ifndef UCL_MQTT_HELPER_H
#define UCL_MQTT_HELPER_H

#include <stdbool.h>

/**
 * @brief Publish node state.
 *
 * @param[in] unid UNID of the node
 * @param[in] available true if node is online
 */
void publish_node_state(char *unid, bool available);

/**
 * @brief Publish asset tag state.
 *
 * @param[in] unid UNID of the asset tag
 */
void publish_tag_state(char *unid);

#endif  // UCL_MQTT_HELPER_H
