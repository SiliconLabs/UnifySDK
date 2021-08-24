
/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ZWAVE_COMMAND_CLASS_THERMOSTAT_MODE_ATTRIBUTES_H
#define ZWAVE_COMMAND_CLASS_THERMOSTAT_MODE_ATTRIBUTES_H

// Includes from other components
#include "sl_status.h"
#include "zwave_controller_types.h"
#include "attribute_store.h"

// Generic incldues
#include <stdbool.h>

// Thermostat Mode Command Class
#define COMMAND_CLASS_THERMOSTAT_MODE 0x40

#define ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE_VERSION \
  ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_THERMOSTAT_MODE)

#define ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE \
  ((attribute_store_type_t)((COMMAND_CLASS_THERMOSTAT_MODE << 8) | 0x02))

#define ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES \
  ((attribute_store_type_t)((COMMAND_CLASS_THERMOSTAT_MODE << 8) | 0x03))

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Verify whether the mode is supported.
 *
 * @param unid
 * @param endpoint
 * @param mode
 *
 * @returns true if supported
 * @returns false if not supported
 */
bool command_class_thermostat_mode_is_mode_supported(
  const char *unid, const zwave_endpoint_id_t endpoint, uint8_t mode);

/**
 * @brief Get the currently configured modes.
 *
 * @param unid
 * @param endpoint
 * @param value_state
 *
 * @returns current modes. 0 in case of error
 */
int32_t command_class_thermostat_mode_get_mode(
  const char *unid,
  const zwave_endpoint_id_t endpoint,
  attribute_store_node_value_state_t value_state);

/**
 * @brief Set the currently configured modes.
 *
 * @param unid
 * @param endpoint
 * @param value_state
 *
 * @returns SL_STATUS_OK if it was written
 */
sl_status_t command_class_thermostat_mode_set_mode(
  const char *unid,
  const zwave_endpoint_id_t endpoint,
  attribute_store_node_value_state_t value_state,
  int32_t new_mode);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_THERMOSTAT_MODE_ATTRIBUTES_H
