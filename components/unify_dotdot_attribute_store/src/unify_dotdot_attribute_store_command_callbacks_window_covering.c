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

/**
 * @brief Finds if WindowCovering lift feature is supported for a UNID/ep
 *
 * @param unid          UNID of the node we want to get the information for
 * @param endpoint      Endpoint of the node we want to get the information for
 * @param percent       check for percentage attribute instead
 * @return bool         true if supported, false if not supported
 */
static bool dotdot_is_supported_window_covering_lift(dotdot_unid_t unid,
                                                     dotdot_endpoint_id_t endpoint,
                                                     bool percent)
{
  // if window covering type implies its a tilt only type then return false
  uint8_t type
    = dotdot_get_window_covering_window_covering_type(unid,
                                                      endpoint,
                                                      REPORTED_ATTRIBUTE);
  if (type == ZCL_WINDOW_COVERING_WINDOW_COVERING_TYPE_SHUTTER
      || type == ZCL_WINDOW_COVERING_WINDOW_COVERING_TYPE_TILT_BLIND_TILT_ONLY)
    return false;
  // else check if lift attribute is supported
  return (
    percent
      ? dotdot_is_supported_window_covering_current_position_lift_percentage(
          unid,
          endpoint)
      : dotdot_is_supported_window_covering_current_position_lift(unid,
                                                                  endpoint));
}

/**
 * @brief Finds if WindowCovering tilt feature is supported for a UNID/ep
 *
 * @param unid          UNID of the node we want to get the information for
 * @param endpoint      Endpoint of the node we want to get the information for
 * @param percent       check for percentage attribute instead
 * @return bool         true if supported, false if not supported
 */
static bool
  dotdot_is_supported_window_covering_tilt(dotdot_unid_t unid,
                                           dotdot_endpoint_id_t endpoint,
                                           bool percent)
{
  // if window covering type implies its not tilt type then return false
  uint8_t type
    = dotdot_get_window_covering_window_covering_type(unid,
                                                      endpoint,
                                                      REPORTED_ATTRIBUTE);
  if (type != ZCL_WINDOW_COVERING_WINDOW_COVERING_TYPE_SHUTTER
      && type != ZCL_WINDOW_COVERING_WINDOW_COVERING_TYPE_TILT_BLIND_TILT_ONLY
      && type
           != ZCL_WINDOW_COVERING_WINDOW_COVERING_TYPE_TILT_BLIND_LIFT_AND_TILT)
    return false;
  // else check if lift attribute is supported
  return (
    percent
      ? dotdot_is_supported_window_covering_current_position_tilt_percentage(
          unid,
          endpoint)
      : dotdot_is_supported_window_covering_current_position_tilt(unid,
                                                                  endpoint));
}

/**
 * @brief Process changes request to mode attribute and takes necessary action.
 *
 * @param node        attribute store node WindowCovering::Mode attribute
 * @param change      type of change 
 */
static void on_mode_update(attribute_store_node_t node,
                           attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }
  uint8_t mode;
  uint8_t u8_size = 1;
  attribute_store_get_node_attribute_value(node,
                                           DESIRED_ATTRIBUTE,
                                           &mode,
                                           &u8_size);
  attribute_store_node_t ep_node = attribute_store_get_first_parent_with_type(
    node,
    unify_dotdot_attributes_endpoint_attribute());
  attribute_store_node_t config_status_node
    = attribute_store_get_first_child_by_type(
      ep_node,
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_CONFIG_OR_STATUS);
  uint8_t value;
    attribute_store_get_node_attribute_value(config_status_node,
                                             REPORTED_ATTRIBUTE,
                                             &value,
                                             &u8_size);

  if (mode & WINDOW_COVERING_MODE_MOTOR_DIRECTION_REVERSED) {

    value |= WINDOW_COVERING_CONFIG_OR_STATUS_OPEN_AND_UP_COMMANDS_REVERSED;
    attribute_store_set_desired(config_status_node, &value, u8_size);
  }
  else if (value & WINDOW_COVERING_CONFIG_OR_STATUS_OPEN_AND_UP_COMMANDS_REVERSED) {
    value ^= WINDOW_COVERING_CONFIG_OR_STATUS_OPEN_AND_UP_COMMANDS_REVERSED;
    attribute_store_set_desired(config_status_node, &value, u8_size);
  }
  if (mode & WINDOW_COVERING_MODE_CALIBRATION_MODE
      || mode & WINDOW_COVERING_MODE_MAINTENANCE_MODE) {
    if (value & WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL) {
      value ^= WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL;
      attribute_store_set_desired(config_status_node, &value, u8_size);
    }
  }
  else if (!(value & WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL))
  {
    value |= WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL;
    attribute_store_set_desired(config_status_node, &value, u8_size);
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

  // operational check
  uint8_t config_status = dotdot_get_window_covering_config_or_status(
    unid,
    endpoint,
    DESIRED_OR_REPORTED_ATTRIBUTE);
  if (!(config_status & WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL))
  {
    sl_log_debug(
      LOG_TAG,
      "config status indication not operational, hence ignore command");
    return SL_STATUS_FAIL;
  }

  if (!is_desired_value_update_on_commands_enabled()) {
    return SL_STATUS_OK;
  }

  sl_log_debug(LOG_TAG,
                "Updating ZCL desired values after "
                "WindowCovering::UpOrOpen command");

  // Adjust CurrentPositionLift
  if (dotdot_is_supported_window_covering_lift(unid, endpoint, false)) {
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

  // Adjust CurrentPositionTilt
  if (dotdot_is_supported_window_covering_tilt(unid, endpoint, false)) {
    uint16_t target_tilt = get_installed_open_limit_tilt(unid, endpoint);
    dotdot_set_window_covering_current_position_tilt(unid,
                                                      endpoint,
                                                      DESIRED_ATTRIBUTE,
                                                      target_tilt);

    if (is_clear_reported_enabled()) {
      sl_log_debug(
        LOG_TAG,
        "Clearing WindowCovering::CurrentPositionTilt reported value");
      dotdot_window_covering_current_position_tilt_undefine_reported(
        unid,
        endpoint);
    }

    // Adjust CurrentPositionTiltPercentage
    if (dotdot_is_supported_window_covering_current_position_tilt_percentage(
          unid,
          endpoint)) {
      dotdot_set_window_covering_current_position_tilt_percentage(
        unid,
        endpoint,
        DESIRED_ATTRIBUTE,
        0);

      if (is_clear_reported_enabled()) {
        sl_log_debug(LOG_TAG,
                      "Clearing WindowCovering::CurrentPositionTiltPercentage "
                      "reported value");
        dotdot_window_covering_current_position_tilt_percentage_undefine_reported(
          unid,
          endpoint);
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

  // operational check
  uint8_t config_status = dotdot_get_window_covering_config_or_status(
    unid,
    endpoint,
    DESIRED_OR_REPORTED_ATTRIBUTE);
  if (!(config_status & WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL))
  {
    sl_log_debug(
      LOG_TAG,
      "config status indication not operational, hence ignore command");
    return SL_STATUS_FAIL;
  }

  // Normal case
  if (!is_desired_value_update_on_commands_enabled()) {
    return SL_STATUS_OK;
  }
  
  sl_log_debug(LOG_TAG,
                "Updating ZCL desired values after "
                "WindowCovering::DownOrClose command");

  // Adjust CurrentPositionLift
  if (dotdot_is_supported_window_covering_lift(unid, endpoint, false)) {
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
  
  // Adjust CurrentPositionTilt
  if (dotdot_is_supported_window_covering_tilt(unid, endpoint, false)) {
    uint16_t target_tilt = get_installed_closed_limit_tilt(unid, endpoint);
    dotdot_set_window_covering_current_position_tilt(unid,
                                                      endpoint,
                                                      DESIRED_ATTRIBUTE,
                                                      target_tilt);

    if (is_clear_reported_enabled()) {
      sl_log_debug(
        LOG_TAG,
        "Clearing WindowCovering::CurrentPositionTilt reported value");
      dotdot_window_covering_current_position_tilt_undefine_reported(
        unid,
        endpoint);
    }

    // Adjust CurrentPositionTiltPercentage
    if (dotdot_is_supported_window_covering_current_position_tilt_percentage(
          unid,
          endpoint)) {
      dotdot_set_window_covering_current_position_tilt_percentage(
        unid,
        endpoint,
        DESIRED_ATTRIBUTE,
        100);

      if (is_clear_reported_enabled()) {
        sl_log_debug(LOG_TAG,
                      "Clearing WindowCovering::CurrentPositionTiltPercentage "
                      "reported value");
        dotdot_window_covering_current_position_tilt_percentage_undefine_reported(
          unid,
          endpoint);
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

  // operational check
  uint8_t config_status = dotdot_get_window_covering_config_or_status(
    unid,
    endpoint,
    DESIRED_OR_REPORTED_ATTRIBUTE);
  if (!(config_status & WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL))
  {
    sl_log_debug(
      LOG_TAG,
      "config status indication not operational, hence ignore command");
    return SL_STATUS_FAIL;
  }

  // Normal case
  if (is_desired_value_update_on_commands_enabled()
      && dotdot_is_supported_window_covering_lift(unid, endpoint, false)) {
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
    // Adjust CurrentPositionLiftPercentage
    if (dotdot_is_supported_window_covering_current_position_lift_percentage(
          unid,
          endpoint)) {
      uint8_t lift_percent
        = ((lift_value - min_value) * 100) / (max_value - min_value);
      dotdot_set_window_covering_current_position_lift_percentage(
        unid,
        endpoint,
        DESIRED_ATTRIBUTE,
        lift_percent);

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

  // operational check
  uint8_t config_status = dotdot_get_window_covering_config_or_status(
    unid,
    endpoint,
    DESIRED_OR_REPORTED_ATTRIBUTE);
  if (!(config_status & WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL))
  {
    sl_log_debug(
      LOG_TAG,
      "config status indication not operational, hence ignore command");
    return SL_STATUS_FAIL;
  }

  // Normal case
  if (is_desired_value_update_on_commands_enabled()
      && dotdot_is_supported_window_covering_lift(unid, endpoint, true)) {
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
    // Adjust CurrentPositionLift
    if (dotdot_is_supported_window_covering_current_position_lift(
          unid,
          endpoint)) {
      uint16_t min_value = get_installed_open_limit_lift(unid, endpoint);
      uint16_t max_value = get_installed_closed_limit_lift(unid, endpoint);
      uint16_t lift
        = ((max_value - min_value) * lift_percentage / 100) + min_value;
      dotdot_set_window_covering_current_position_lift(
        unid,
        endpoint,
        DESIRED_ATTRIBUTE,
        lift);

      if (is_clear_reported_enabled()) {
        sl_log_debug(LOG_TAG,
                      "Clearing WindowCovering::CurrentPositionLift "
                      "reported value");
        dotdot_window_covering_current_position_lift_undefine_reported(
          unid,
          endpoint);
      }
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

  // operational check
  uint8_t config_status = dotdot_get_window_covering_config_or_status(
    unid,
    endpoint,
    DESIRED_OR_REPORTED_ATTRIBUTE);
  if (!(config_status & WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL))
  {
    sl_log_debug(
      LOG_TAG,
      "config status indication not operational, hence ignore command");
    return SL_STATUS_FAIL;
  }

  // Normal case
  if (is_desired_value_update_on_commands_enabled()
      && dotdot_is_supported_window_covering_tilt(unid, endpoint, false)) {
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
    // Adjust CurrentPositionTiltPercentage
    if (dotdot_is_supported_window_covering_current_position_tilt_percentage(
          unid,
          endpoint)) {
      uint8_t tilt_percent
        = ((tilt_value - min_value) * 100) / (max_value - min_value);
      dotdot_set_window_covering_current_position_tilt_percentage(
        unid,
        endpoint,
        DESIRED_ATTRIBUTE,
        tilt_percent);

      if (is_clear_reported_enabled()) {
        sl_log_debug(LOG_TAG,
                      "Clearing WindowCovering::CurrentPositionTiltPercentage "
                      "reported value");
        dotdot_window_covering_current_position_tilt_percentage_undefine_reported(
          unid,
          endpoint);
      }
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

  // operational check
  uint8_t config_status = dotdot_get_window_covering_config_or_status(
    unid,
    endpoint,
    DESIRED_OR_REPORTED_ATTRIBUTE);
  if (!(config_status & WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL))
  {
    sl_log_debug(
      LOG_TAG,
      "config status indication not operational, hence ignore command");
    return SL_STATUS_FAIL;
  }

  // Normal case
  if (is_desired_value_update_on_commands_enabled()
      && dotdot_is_supported_window_covering_tilt(unid, endpoint, true)) {
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
    // Adjust CurrentPositionTilt
    if (dotdot_is_supported_window_covering_current_position_tilt(
          unid,
          endpoint)) {
      uint16_t min_value = get_installed_open_limit_tilt(unid, endpoint);
      uint16_t max_value = get_installed_closed_limit_tilt(unid, endpoint);
      uint16_t tilt
        = ((max_value - min_value) * tilt_percentage / 100) + min_value;
      dotdot_set_window_covering_current_position_tilt(
        unid,
        endpoint,
        DESIRED_ATTRIBUTE,
        tilt);

      if (is_clear_reported_enabled()) {
        sl_log_debug(LOG_TAG,
                      "Clearing WindowCovering::CurrentPositionTilt "
                      "reported value");
        dotdot_window_covering_current_position_tilt_undefine_reported(
          unid,
          endpoint);
      }
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

  attribute_store_register_callback_by_type_and_state(
    &on_mode_update,
    DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_MODE,
    DESIRED_ATTRIBUTE);
}
