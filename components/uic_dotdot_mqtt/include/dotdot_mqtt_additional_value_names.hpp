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
 * @defgroup dotdot_mqtt_additional_value_names DotDot MQTT Value names for attributes not covered automatically by ZCL
 * @ingroup dotdot_mqtt
 * @brief Additional string/value pairs for \ref dotdot_mqtt
 *
 * @{
 */

#ifndef DOTDOT_MQTT_ADDITIONAL_VALUE_NAMES_H
#define DOTDOT_MQTT_ADDITIONAL_VALUE_NAMES_H

#include <stdbool.h>
#include <string>
#include "uic_typedefs.h"

/**
 * @brief Checks if a Cluster ID / Attribute ID / value has a name
 *
 * @param cluster_id        Cluster ID, for example 0x0006 or DOTDOT_ON_OFF_CLUSTER_ID
 * @param attribute_id      Attribute ID, for example 0x0000 or DOTDOT_ON_OFF_ON_OFF_ATTRIBUTE_ID
 * @param value             Value for the attribute ID
 * @returns true if the value is special and has a name, false otherwise
 */
bool uic_dotdot_has_attribute_value_a_name(dotdot_cluster_id_t cluster_id,
                                           dotdot_attribute_id_t attribute_id,
                                           int64_t value);

/**
 * @brief Checks if a Cluster ID / Attribute ID / value has a name
 *
 * @param cluster_id        Cluster ID, for example 0x0006 or DOTDOT_ON_OFF_CLUSTER_ID
 * @param attribute_id      Attribute ID, for example 0x0000 or DOTDOT_ON_OFF_ON_OFF_ATTRIBUTE_ID
 * @param value             Value for the attribute ID
 * @returns String with the name of the value. Empty string in case of error
 */
std::string
  uic_dotdot_get_attribute_value_name(dotdot_cluster_id_t cluster_id,
                                      dotdot_attribute_id_t attribute_id,
                                      int64_t value);

#endif  //DOTDOT_MQTT_ADDITIONAL_VALUE_NAMES_H
/** @} end dotdot_mqtt_additional_value_names */
