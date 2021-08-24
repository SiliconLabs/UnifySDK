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
#include "thermostat_cluster_mapper.h"
#include "dotdot_attributes.h"

// Includes from other components
#include "sl_log.h"
#include "sl_status.h"

// Generic includes
#include <assert.h>
#include <string>

// Includes from auto-generated files
#include "dotdot_mqtt.h"
#include "attribute.hpp"
// Setup Log ID
constexpr char LOG_TAG[] = "thermostat_cluster_mapper";

using namespace attribute_store;

///////////////////////////////////////////////////////////////////////////////
// Forward decalarations
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Z-Wave Attribute callback functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Control helper functions
///////////////////////////////////////////////////////////////////////////////
static void increase_current_heating_setpoint(const char *unid,
                                              const uint8_t endpoint,
                                              int32_t amount)
{
  int32_t current_zigbee_setpoint
    = dotdot_get_thermostat_occupied_heating_setpoint(unid,
                                                      endpoint,
                                                      REPORTED_ATTRIBUTE);

  // Increment the zigbee part.
  current_zigbee_setpoint += amount;

  // Update the DESIRED value on ZigBee:
  dotdot_set_thermostat_occupied_heating_setpoint(unid,
                                                  endpoint,
                                                  DESIRED_ATTRIBUTE,
                                                  current_zigbee_setpoint);
}

static void increase_current_cooling_setpoint(const char *unid,
                                              const uint8_t endpoint,
                                              int32_t amount)
{
  int32_t current_zigbee_setpoint
    = dotdot_get_thermostat_occupied_cooling_setpoint(unid,
                                                      endpoint,
                                                      REPORTED_ATTRIBUTE);

  // Increment the zigbee part.
  current_zigbee_setpoint += amount;

  // Update the DESIRED value on ZigBee:
  dotdot_set_thermostat_occupied_cooling_setpoint(unid,
                                                  endpoint,
                                                  DESIRED_ATTRIBUTE,
                                                  current_zigbee_setpoint);
}

///////////////////////////////////////////////////////////////////////////////
// ZigBee command handler
///////////////////////////////////////////////////////////////////////////////
static sl_status_t thermostat_cluster_mapper_setpoint_raise_or_lower_callback(
  const char *unid,
  const uint8_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type,
  SetpointRaiseOrLowerMode mode,
  int8_t amount)
{
  if (callback_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (dotdot_is_supported_thermostat_occupied_heating_setpoint(unid,
                                                                     endpoint)
            || dotdot_is_supported_thermostat_occupied_cooling_setpoint(
              unid,
              endpoint))
             ? SL_STATUS_OK
             : SL_STATUS_FAIL;
  }
  sl_log_debug(
    LOG_TAG,
    "Incoming raise/lower temperature command. Mode: %d, Amount: %d (deci C)",
    mode,
    amount);
  // ZCL Spec specifies SetpointRaiseOrLower amount to be in deci (1/10)C,
  // but the setpoints are in centi (1/100)C.
  const int32_t amount_x_10 = static_cast<int32_t>(amount) * 10;

  // Adjust our setpoints, if supported
  switch (mode) {
    case 0x00:  // Heat
      if (dotdot_is_supported_thermostat_occupied_heating_setpoint(unid,
                                                                   endpoint)) {
        increase_current_heating_setpoint(unid, endpoint, amount_x_10);
      }
      break;
    case 0x01:  // Heat
      if (dotdot_is_supported_thermostat_occupied_cooling_setpoint(unid,
                                                                   endpoint)) {
        increase_current_cooling_setpoint(unid, endpoint, amount_x_10);
      }
      break;
    case 0x02:  //both Heat and Cool
      if (dotdot_is_supported_thermostat_occupied_heating_setpoint(unid,
                                                                   endpoint)) {
        increase_current_heating_setpoint(unid, endpoint, amount_x_10);
      }
      if (dotdot_is_supported_thermostat_occupied_cooling_setpoint(unid,
                                                                   endpoint)) {
        increase_current_cooling_setpoint(unid, endpoint, amount_x_10);
      }
      break;
    default:
      // Do nothing.
      break;
  }

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
bool thermostat_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "Thermostat cluster mapper initialization");

  // Register callbacks to supported commands
  uic_mqtt_dotdot_thermostat_setpoint_raise_or_lower_callback_set(
    thermostat_cluster_mapper_setpoint_raise_or_lower_callback);

  return true;
}

int thermostat_cluster_mapper_teardown()
{
  sl_log_debug(LOG_TAG, "Teardown of the Thermostat cluster mapper");
  return 0;
}
