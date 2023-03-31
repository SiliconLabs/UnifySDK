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
 * @defgroup level_cluster_mapper Level Cluster Mapper
 * @ingroup dotdot_mapper
 * @brief Maps Level Cluster incoming Commands to attribute modifications.
 *
 * This module is used to register callbacks for incoming Level Cluster Commands
 * and update the corresponding ZCL attributes in the @ref attribute_store .
 * @{
 */

#ifndef LEVEL_CLUSTER_MAPPER_H
#define LEVEL_CLUSTER_MAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dotdot_mqtt.h"

/**
 * @brief Intitialise the Level cluster mapper
 *
 */
void level_cluster_mapper_init(void);

/**
 * @brief Clears the Level cluster mapper from all its content.
 */
void level_cluster_mapper_teardown(void);

/**
 * Move to level command used by the on/off cluster
 */
sl_status_t level_cluster_mapper_move_to_level_with_on_off(
  dotdot_unid_t unid,
  uint8_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t level,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override);

sl_status_t level_cluster_mapper_move_to_level(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t level,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override);

#ifdef __cplusplus
}
#endif

#endif  //LEVEL_CLUSTER_MAPPER_H
/** @} end level_cluster_mapper */
