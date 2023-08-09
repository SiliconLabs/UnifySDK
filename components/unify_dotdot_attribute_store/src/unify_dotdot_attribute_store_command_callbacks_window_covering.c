/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "unify_dotdot_attribute_store_command_callbacks.h"
#include "unify_dotdot_attribute_store_command_callbacks_window_covering.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_attribute_store_configuration.h"
#include "unify_dotdot_defined_attribute_types.h"

// Other Unify components.
#include "dotdot_mqtt.h"
#include "sl_log.h"

#define LOG_TAG \
  "unify_dotdot_attribute_store_window_covering_commands_callbacks"

#define ATTRIBUTE(type) DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_##type

// WindowCovering cluster constants
const uint16_t DEFAULT_INSTALLED_OPEN_LIMIT_LIFT   = 0x00;
const uint16_t DEFAULT_INSTALLED_CLOSED_LIMIT_LIFT = 65535;
const uint16_t DEFAULT_INSTALLED_OPEN_LIMIT_TILT   = 0x00;
const uint16_t DEFAULT_INSTALLED_CLOSED_LIMIT_TILT = 65535;

/**
 * @brief Finds the WindowCovering::InstalledOpenLimitLift value for a UNID/ep
 *
 * @param unid          UNID of the node we want to get the information for
 * @param endpoint      Endpoint of the node we want to get the information for
 * @return uint16 Value for the, default if does not exist
 */
static uint16_t get_installed_open_limit_lift(dotdot_unid_t unid,
                                              dotdot_endpoint_id_t endpoint)
{
  if (dotdot_is_supported_window_covering_installed_open_limit_lift(unid,
                                                                    endpoint)) {
    return dotdot_get_window_covering_installed_open_limit_lift(
      unid,
      endpoint,
      REPORTED_ATTRIBUTE);
  } else {
    return DEFAULT_INSTALLED_OPEN_LIMIT_LIFT;
  }
}

/**
 * @brief Finds the WindowCovering::InstalledClosedLimitLift value for a UNID/ep
 *
 * @param unid          UNID of the node we want to get the information for
 * @param endpoint      Endpoint of the node we want to get the information for
 * @return uint16 Value for the, default if does not exist
 */
static uint16_t get_installed_closed_limit_lift(dotdot_unid_t unid,
                                                dotdot_endpoint_id_t endpoint)
{
  if (dotdot_is_supported_window_covering_installed_closed_limit_lift(
        unid,
        endpoint)) {
    return dotdot_get_window_covering_installed_closed_limit_lift(
      unid,
      endpoint,
      REPORTED_ATTRIBUTE);
  } else {
    return DEFAULT_INSTALLED_CLOSED_LIMIT_LIFT;
  }
}

/**
 * @brief Finds the WindowCovering::InstalledOpenLimitTilt value for a UNID/ep
 *
 * @param unid          UNID of the node we want to get the information for
 * @param endpoint      Endpoint of the node we want to get the information for
 * @return uint16 Value for the, default if does not exist
 */
static uint16_t get_installed_open_limit_tilt(dotdot_unid_t unid,
                                              dotdot_endpoint_id_t endpoint)
{
  if (dotdot_is_supported_window_covering_installed_open_limit_tilt(unid,
                                                                    endpoint)) {
    return dotdot_get_window_covering_installed_open_limit_tilt(
      unid,
      endpoint,
      REPORTED_ATTRIBUTE);
  } else {
    return DEFAULT_INSTALLED_OPEN_LIMIT_TILT;
  }
}

/**
 * @brief Finds the WindowCovering::InstalledClosedLimitTilt value for a UNID/ep
 *
 * @param unid          UNID of the node we want to get the information for
 * @param endpoint      Endpoint of the node we want to get the information for
 * @return uint16 Value for the, default if does not exist
 */
static uint16_t get_installed_closed_limit_tilt(dotdot_unid_t unid,
                                                dotdot_endpoint_id_t endpoint)
{
  if (dotdot_is_supported_window_covering_installed_closed_limit_tilt(
        unid,
        endpoint)) {
    return dotdot_get_window_covering_installed_closed_limit_tilt(
      unid,
      endpoint,
      REPORTED_ATTRIBUTE);
  } else {
    return DEFAULT_INSTALLED_CLOSED_LIMIT_TILT;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Incoming command/control functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  up_or_open_command(dotdot_unid_t unid,
                     dotdot_endpoint_id_t endpoint,
                     uic_mqtt_dotdot_callback_call_type_t call_type)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_window_covering_window_covering_type(unid,
                                                                      endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "WindowCovering::UpOrOpen command");

    // Adjust CurrentPositionLift
    if (dotdot_is_supported_window_covering_current_position_lift(unid,
                                                                  endpoint)) {
      uint16_t target_lift = get_installed_open_limit_lift(unid, endpoint);
      dotdot_set_window_covering_current_position_lift(unid,
                                                       endpoint,
                                                       DESIRED_ATTRIBUTE,
                                                       target_lift);

      if (is_clear_reported_enabled()) {
        sl_log_debug(
          LOG_TAG,
          "Clearing WindowCovering::CurrentPositionLift reported value");
        dotdot_window_covering_current_position_lift_undefine_reported(
          unid,
          endpoint);
      }

      // Adjust CurrentPositionLiftPercentage
      if (dotdot_is_supported_window_covering_current_position_lift_percentage(
            unid,
            endpoint)) {
        dotdot_set_window_covering_current_position_lift_percentage(
          unid,
          endpoint,
          DESIRED_ATTRIBUTE,
          0);

        if (is_clear_reported_enabled()) {
          sl_log_debug(LOG_TAG,
                       "Clearing WindowCovering::CurrentPositionLiftPercentage "
                       "reported value");
          dotdot_window_covering_current_position_lift_percentage_undefine_reported(
            unid,
            endpoint);
        }
      }
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t
  down_or_close_command(dotdot_unid_t unid,
                        dotdot_endpoint_id_t endpoint,
                        uic_mqtt_dotdot_callback_call_type_t call_type)
{
  // Support check
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_window_covering_window_covering_type(unid,
                                                                      endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Normal case
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "WindowCovering::DownOrClose command");

    // Adjust CurrentPositionLift
    if (dotdot_is_supported_window_covering_current_position_lift(unid,
                                                                  endpoint)) {
      uint16_t target_lift = get_installed_closed_limit_lift(unid, endpoint);
      dotdot_set_window_covering_current_position_lift(unid,
                                                       endpoint,
                                                       DESIRED_ATTRIBUTE,
                                                       target_lift);

      if (is_clear_reported_enabled()) {
        sl_log_debug(
          LOG_TAG,
          "Clearing WindowCovering::CurrentPositionLift reported value");
        dotdot_window_covering_current_position_lift_undefine_reported(
          unid,
          endpoint);
      }

      // Adjust CurrentPositionLiftPercentage
      if (dotdot_is_supported_window_covering_current_position_lift_percentage(
            unid,
            endpoint)) {
        dotdot_set_window_covering_current_position_lift_percentage(
          unid,
          endpoint,
          DESIRED_ATTRIBUTE,
          100);

        if (is_clear_reported_enabled()) {
          sl_log_debug(LOG_TAG,
                       "Clearing WindowCovering::CurrentPositionLiftPercentage "
                       "reported value");
          dotdot_window_covering_current_position_lift_percentage_undefine_reported(
            unid,
            endpoint);
        }
      }
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t stop_command(dotdot_unid_t unid,
                                dotdot_endpoint_id_t endpoint,
                                uic_mqtt_dotdot_callback_call_type_t call_type)
{
  // Support check
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_window_covering_window_covering_type(unid,
                                                                      endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Normal case
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "WindowCovering::Stop command");

    // Adjust CurrentPositionLift
    attribute_store_node_t endpoint_node
      = unify_dotdot_attributes_get_endpoint_node()(unid, endpoint);
    attribute_store_node_t node = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE(CURRENT_POSITION_LIFT));
    attribute_store_set_desired_as_reported(node);

    if (is_clear_reported_enabled()) {
      sl_log_debug(
        LOG_TAG,
        "Clearing WindowCovering::CurrentPositionLift reported value");
      attribute_store_undefine_reported(node);
    }

    // CurrentPositionTilt
    node = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE(CURRENT_POSITION_TILT));
    attribute_store_set_desired_as_reported(node);

    if (is_clear_reported_enabled()) {
      sl_log_debug(
        LOG_TAG,
        "Clearing WindowCovering::CurrentPositionTilt reported value");
      attribute_store_undefine_reported(node);
    }

    // CurrentPositionLiftPercentage
    node = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE(CURRENT_POSITION_LIFT_PERCENTAGE));
    attribute_store_set_desired_as_reported(node);

    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing WindowCovering::CurrentPositionLiftPercentage "
                   "reported value");
      attribute_store_undefine_reported(node);
    }

    // CurrentPositionTiltPercentage
    node = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE(CURRENT_POSITION_TILT_PERCENTAGE));
    attribute_store_set_desired_as_reported(node);

    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing WindowCovering::CurrentPositionTiltPercentage "
                   "reported value");
      attribute_store_undefine_reported(node);
    }

  }  // is_desired_value_update_on_commands_enabled()
  return SL_STATUS_OK;
}

static sl_status_t
  go_to_lift_value_command(dotdot_unid_t unid,
                           dotdot_endpoint_id_t endpoint,
                           uic_mqtt_dotdot_callback_call_type_t call_type,
                           uint16_t lift_value)
{
  // Support check
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_window_covering_current_position_lift(unid,
                                                                       endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Normal case
  if (is_desired_value_update_on_commands_enabled()
      && dotdot_is_supported_window_covering_current_position_lift(unid,
                                                                   endpoint)) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "WindowCovering::GoToLiftValue command");

    uint16_t min_value = get_installed_open_limit_lift(unid, endpoint);
    uint16_t max_value = get_installed_closed_limit_lift(unid, endpoint);
    if (lift_value < min_value) {
      lift_value = min_value;
    } else if (lift_value > max_value) {
      lift_value = max_value;
    }
    dotdot_set_window_covering_current_position_lift(unid,
                                                     endpoint,
                                                     DESIRED_ATTRIBUTE,
                                                     lift_value);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing WindowCovering::CurrentPositionLift "
                   "reported value");
      dotdot_window_covering_current_position_lift_undefine_reported(unid,
                                                                     endpoint);
    }
  }
  return SL_STATUS_OK;
}

static sl_status_t
  go_to_lift_percentage_command(dotdot_unid_t unid,
                                dotdot_endpoint_id_t endpoint,
                                uic_mqtt_dotdot_callback_call_type_t call_type,
                                uint8_t lift_percentage)
{
  // Support check
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_window_covering_current_position_lift_percentage(
               unid,
               endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Normal case
  if (is_desired_value_update_on_commands_enabled()
      && dotdot_is_supported_window_covering_current_position_lift_percentage(
        unid,
        endpoint)) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "WindowCovering::GoToLiftPercentage command");

    if (lift_percentage > 100) {
      lift_percentage = 100;
    }
    dotdot_set_window_covering_current_position_lift_percentage(
      unid,
      endpoint,
      DESIRED_ATTRIBUTE,
      lift_percentage);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing WindowCovering::CurrentPositionLiftPercentage "
                   "reported value");
      dotdot_window_covering_current_position_lift_percentage_undefine_reported(
        unid,
        endpoint);
    }
  }
  return SL_STATUS_OK;
}

static sl_status_t
  go_to_tilt_value_command(dotdot_unid_t unid,
                           dotdot_endpoint_id_t endpoint,
                           uic_mqtt_dotdot_callback_call_type_t call_type,
                           uint16_t tilt_value)
{
  // Support check
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_window_covering_current_position_tilt(unid,
                                                                       endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Normal case
  if (is_desired_value_update_on_commands_enabled()
      && dotdot_is_supported_window_covering_current_position_tilt(unid,
                                                                   endpoint)) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "WindowCovering::GoToTiltValue command");

    uint16_t min_value = get_installed_open_limit_tilt(unid, endpoint);
    uint16_t max_value = get_installed_closed_limit_tilt(unid, endpoint);
    if (tilt_value < min_value) {
      tilt_value = min_value;
    } else if (tilt_value > max_value) {
      tilt_value = max_value;
    }
    dotdot_set_window_covering_current_position_tilt(unid,
                                                     endpoint,
                                                     DESIRED_ATTRIBUTE,
                                                     tilt_value);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing WindowCovering::CurrentPositionTilt "
                   "reported value");
      dotdot_window_covering_current_position_tilt_undefine_reported(unid,
                                                                     endpoint);
    }
  }
  return SL_STATUS_OK;
}

static sl_status_t
  go_to_tilt_percentage_command(dotdot_unid_t unid,
                                dotdot_endpoint_id_t endpoint,
                                uic_mqtt_dotdot_callback_call_type_t call_type,
                                uint8_t tilt_percentage)
{
  // Support check
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_window_covering_current_position_tilt_percentage(
               unid,
               endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Normal case
  if (is_desired_value_update_on_commands_enabled()
      && dotdot_is_supported_window_covering_current_position_tilt_percentage(
        unid,
        endpoint)) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "WindowCovering::GoToTiltPercentage command");

    if (tilt_percentage > 100) {
      tilt_percentage = 100;
    }
    dotdot_set_window_covering_current_position_tilt_percentage(
      unid,
      endpoint,
      DESIRED_ATTRIBUTE,
      tilt_percentage);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing WindowCovering::CurrentPositionTiltPercentage "
                   "reported value");
      dotdot_window_covering_current_position_tilt_percentage_undefine_reported(
        unid,
        endpoint);
    }
  }
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
void window_covering_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "WindowCovering cluster mapper initialization");

  uic_mqtt_dotdot_window_covering_up_or_open_callback_set(&up_or_open_command);
  uic_mqtt_dotdot_window_covering_down_or_close_callback_set(
    &down_or_close_command);
  uic_mqtt_dotdot_window_covering_stop_callback_set(&stop_command);
  uic_mqtt_dotdot_window_covering_go_to_lift_value_callback_set(
    &go_to_lift_value_command);
  uic_mqtt_dotdot_window_covering_go_to_lift_percentage_callback_set(
    &go_to_lift_percentage_command);
  uic_mqtt_dotdot_window_covering_go_to_tilt_value_callback_set(
    &go_to_tilt_value_command);
  uic_mqtt_dotdot_window_covering_go_to_tilt_percentage_callback_set(
    &go_to_tilt_percentage_command);
}
