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

// Includes from this component
#include "zwave_command_class_thermostat_mode_attributes.h"

// Includes from other components
#include "sl_log.h"
#include "sl_status.h"
#include "ZW_classcmd.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_helper.h"
#include "attribute_store.h"

// Generic includes
#include <assert.h>
#include <stdint.h>

// Log define
#define LOG_TAG "zwave_command_class_thermostat_mode_attributes"

bool command_class_thermostat_mode_is_mode_supported(
  const char *unid, const zwave_endpoint_id_t endpoint, uint8_t mode)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  attribute_store_node_t supported_modes_node
    = attribute_store_get_node_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES,
      0);

  uint16_t supported_modes = 0;
  attribute_store_read_value(supported_modes_node,
                             REPORTED_ATTRIBUTE,
                             &supported_modes,
                             sizeof(supported_modes));

  if (supported_modes & (1 << mode)) {
    return true;
  }

  sl_log_debug(LOG_TAG,
               "Thermostat Mode %d is not supported by %s, endpoint %d",
               mode,
               unid,
               endpoint);
  return false;
}

int32_t command_class_thermostat_mode_get_mode(
  const char *unid,
  const zwave_endpoint_id_t endpoint,
  attribute_store_node_value_state_t value_state)
{
  int32_t thermostat_mode = 0;

  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  attribute_store_node_t thermostat_mode_node
    = attribute_store_get_node_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE,
      0);

  attribute_store_read_value(thermostat_mode_node,
                             value_state,
                             &thermostat_mode,
                             sizeof(thermostat_mode));

  return thermostat_mode;
}

sl_status_t command_class_thermostat_mode_set_mode(
  const char *unid,
  const zwave_endpoint_id_t endpoint,
  attribute_store_node_value_state_t value_state,
  int32_t new_mode)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  attribute_store_node_t thermostat_mode_node
    = attribute_store_get_node_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE,
      0);

  return attribute_store_set_node_attribute_value(thermostat_mode_node,
                                                  value_state,
                                                  (uint8_t *)&new_mode,
                                                  sizeof(new_mode));
}
