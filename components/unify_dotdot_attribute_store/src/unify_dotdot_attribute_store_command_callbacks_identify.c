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
#include "unify_dotdot_attribute_store_command_callbacks_identify.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_attribute_store_configuration.h"
#include "unify_dotdot_defined_attribute_types.h"

// Includes from Unify
#include "sl_log.h"
#include "sl_status.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_transitions.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_generated_commands.h"

// Setup Log ID
#define LOG_TAG "unify_dotdot_attribute_store_identify_commands_callbacks"

////////////////////////////////////////////////////////////////////////////////
// Attribute Store callbacks
////////////////////////////////////////////////////////////////////////////////
static void
  on_identify_time_reported_update(attribute_store_node_t updated_node,
                                   attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }
  if (false == attribute_store_is_reported_defined(updated_node)) {
    return;
  }

  // If the reported is set to a non-zero value, make sure to initiate a countdown.
  number_t identify_time = attribute_store_get_reported_number(updated_node);
  if (identify_time > 0) {
    // Attribute is in seconds, so decrement by 1 every second
    attribute_start_fixed_transition(updated_node,
                                     REPORTED_ATTRIBUTE,
                                     0,
                                     -1,
                                     1000);
  } else {
    attribute_store_undefine_desired(updated_node);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Command callback functions for DotDot MQTT
////////////////////////////////////////////////////////////////////////////////
static sl_status_t on_identify(dotdot_unid_t unid,
                               dotdot_endpoint_id_t endpoint,
                               uic_mqtt_dotdot_callback_call_type_t call_type,
                               uint16_t identify_time)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_identify_identify_time(unid, endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  if (dotdot_is_supported_identify_identify_time(unid, endpoint)
      && (is_desired_value_update_on_commands_enabled())) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "Identify:Identify command");
    dotdot_set_identify_identify_time(unid,
                                      endpoint,
                                      DESIRED_ATTRIBUTE,
                                      identify_time);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing Identify:IdentifyTime reported value");
      dotdot_identify_identify_time_undefine_reported(unid, endpoint);
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t on_query(dotdot_unid_t unid,
                            dotdot_endpoint_id_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_identify_identify_time(unid, endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  if (dotdot_is_supported_identify_identify_time(unid, endpoint)
      && (is_desired_value_update_on_commands_enabled())) {
    uint16_t identify_time = 0;
    identify_time = dotdot_get_identify_identify_time(unid,endpoint,
                                                      REPORTED_ATTRIBUTE);
    if (identify_time == 0) {
      return SL_STATUS_OK;
    }
    uic_mqtt_dotdot_identify_command_identify_query_response_fields_t fields = {};
    fields.timeout = identify_time;
    uic_mqtt_dotdot_identify_publish_generated_identify_query_response_command(unid,endpoint,
    &fields);
  }

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
void identify_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "Identify cluster mapper initialization");
  uic_mqtt_dotdot_identify_identify_callback_set(on_identify);
  uic_mqtt_dotdot_identify_identify_query_callback_set(on_query);
  attribute_store_register_callback_by_type_and_state(
    &on_identify_time_reported_update,
    DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
    REPORTED_ATTRIBUTE);
}

void identify_cluster_mapper_teardown()
{
  sl_log_debug(LOG_TAG, "Teardown of the Identify cluster mapper");
}
