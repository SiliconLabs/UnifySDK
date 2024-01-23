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
#include "user_code_cluster_server.h"
#include "zcl_cluster_servers_helpers.hpp"

// Interfaces
#include "zwave_command_class_user_code_types.h"

// ZPC includes
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_command_class_generic_types.h"
#include "attribute_store_defined_attribute_types.h"

// Includes from Unify shared components
#include "attribute.hpp"
#include "attribute_store_helper.h"
#include "sl_log.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"
#include "zap-types.h"

// Generic includes
#include <string>
#include <vector>
#include <stdlib.h>

//using namespace attribute_store;

// Setup Log ID
constexpr char LOG_TAG[] = "user_code_cluster_server";

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_USER_CODE_##type

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
//////////////////////////////////////////////////////////////////////////////
static attribute_store_node_t
  get_user_code_data_node(dotdot_unid_t unid, dotdot_endpoint_id_t endpoint)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);
  return attribute_store_get_first_child_by_type(endpoint_node,
                                                 ATTRIBUTE(DATA));
}
///////////////////////////////////////////////////////////////////////////////
// DotDot MQTT incoming commands handling functions
//////////////////////////////////////////////////////////////////////////////
static sl_status_t
  clear_all_pin_codes_command(dotdot_unid_t unid,
                              dotdot_endpoint_id_t endpoint,
                              uic_mqtt_dotdot_callback_call_type_t call_type)
{
  attribute_store_node_t user_code_data
    = get_user_code_data_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    return attribute_store_node_exists(user_code_data) ? SL_STATUS_OK
                                                       : SL_STATUS_FAIL;
  }

  const command_status_values_t status = NEEDS_ONE_COMMAND;
  return attribute_store_set_child_desired_only_if_exists(
    user_code_data,
    ATTRIBUTE(DELETE_ALL_REQUESTED),
    &status,
    sizeof(status));
}

static sl_status_t
  set_pin_code_command(dotdot_unid_t unid,
                       dotdot_endpoint_id_t endpoint,
                       uic_mqtt_dotdot_callback_call_type_t call_type,
                       DrlkPINUserID userid,
                       DrlkSettableUserStatus user_status,
                       DrlkUserType user_type,
                       const char *pin)
{
  attribute_store_node_t user_code_data
    = get_user_code_data_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    return attribute_store_node_exists(user_code_data) ? SL_STATUS_OK
                                                       : SL_STATUS_FAIL;
  }

  switch (user_type) {
    case ZCL_DRLK_USER_TYPE_UNRESTRICTED_USER: {
      uint16_t user_id = userid;
      attribute_store_node_t user_id_node
        = attribute_store_get_node_child_by_value(user_code_data,
                                                  ATTRIBUTE(USER_ID),
                                                  REPORTED_ATTRIBUTE,
                                                  (uint8_t *)&user_id,
                                                  sizeof(uint16_t),
                                                  0);
      uint8_t user_id_status = user_status;
      if (user_id_status == ZCL_DRLK_USER_STATUS_OCCUPIED_DISABLED) {
        user_id_status = USER_STATUS_DISABLED;
      }
      attribute_store_set_child_desired(user_id_node,
                                        ATTRIBUTE(USER_ID_STATUS),
                                        &user_id_status,
                                        sizeof(user_id_status));
      attribute_store_node_t user_code_node
        = attribute_store_get_first_child_by_type(user_id_node,
                                                  ATTRIBUTE(CODE));
      return attribute_store_set_desired_string(user_code_node, pin);
    }

    case ZCL_DRLK_USER_TYPE_MASTER_USER: {
      attribute_store_node_t admin_code_node
        = attribute_store_get_first_child_by_type(user_code_data,
                                                  ATTRIBUTE(ADMIN_CODE));
      return attribute_store_set_desired_string(admin_code_node, pin);
    }
    default:
      return SL_STATUS_FAIL;
  }
}

static sl_status_t
  clear_pin_code_command(dotdot_unid_t unid,
                         dotdot_endpoint_id_t endpoint,
                         uic_mqtt_dotdot_callback_call_type_t call_type,
                         DrlkPINUserID userid)
{
  attribute_store_node_t user_code_data
    = get_user_code_data_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    return attribute_store_node_exists(user_code_data) ? SL_STATUS_OK
                                                       : SL_STATUS_FAIL;
  }

  uint16_t user_id = userid;
  attribute_store_node_t user_id_node
    = attribute_store_get_node_child_by_value(user_code_data,
                                              ATTRIBUTE(USER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&user_id,
                                              sizeof(uint16_t),
                                              0);

  attribute_store_node_t user_code_node
    = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  attribute_store_undefine_desired(user_code_node);

  const uint8_t user_id_status = USER_STATUS_AVAILABLE;
  return attribute_store_set_child_desired(user_id_node,
                                           ATTRIBUTE(USER_ID_STATUS),
                                           &user_id_status,
                                           sizeof(user_id_status));
}

static sl_status_t
  get_pin_code_command(dotdot_unid_t unid,
                       dotdot_endpoint_id_t endpoint,
                       uic_mqtt_dotdot_callback_call_type_t call_type,
                       DrlkPINUserID userid)
{
  attribute_store_node_t user_code_data
    = get_user_code_data_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    return attribute_store_node_exists(user_code_data) ? SL_STATUS_OK
                                                       : SL_STATUS_FAIL;
  }

  uint16_t user_id = userid;
  attribute_store_node_t user_id_node
    = attribute_store_get_node_child_by_value(user_code_data,
                                              ATTRIBUTE(USER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&user_id,
                                              sizeof(uint16_t),
                                              0);
  uint16_t number_of_users = 0;
  attribute_store_get_child_reported(user_code_data,
                                     ATTRIBUTE(NUMBER_OF_USERS),
                                     &number_of_users,
                                     sizeof(number_of_users));

  if ((user_id > 0) && (user_id <= number_of_users)) {
    if (user_id_node == ATTRIBUTE_STORE_INVALID_NODE) {
      user_id_node = attribute_store_emplace(user_code_data,
                                             ATTRIBUTE(USER_ID),
                                             &user_id,
                                             sizeof(user_id));
    }
  } else {
    return SL_STATUS_FAIL;
  }

  attribute_store_node_t user_code_node
    = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  attribute_store_undefine_desired(user_code_node);
  attribute_store_undefine_reported(user_code_node);

  return attribute_store_node_exists(user_code_node) ? SL_STATUS_OK
                                                     : SL_STATUS_FAIL;
}

/**
 * @brief Checks if the User ID node matches what we expect and undefines
 * the User Code data underneath.
 *
 * @param user_id_node  Attribute Store node for the User ID.
 */
static void undefine_user_code(attribute_store_node_t user_id_node)
{
  if (attribute_store_get_node_type(user_id_node) != ATTRIBUTE(USER_ID)) {
    return;
  }

  // Set the user code to nothing:
  attribute_store_node_t user_code_node
    = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));

  attribute_store_undefine_desired(user_code_node);
  attribute_store_undefine_reported(user_code_node);

  attribute_store_node_t user_id_status_node
    = attribute_store_get_first_child_by_type(user_id_node,
                                              ATTRIBUTE(USER_ID_STATUS));

  attribute_store_undefine_desired(user_id_status_node);
  attribute_store_undefine_reported(user_id_status_node);
}

/**
 * @brief Navigates under the endpoint node and undefines all the
 * known user code data.
 *
 * @param node  Attribute Store node for the Endpoint.
 */
static void undefine_all_user_codes(attribute_store_node_t endpoint_node)
{
  attribute_store_node_t data_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(DATA));

  attribute_store_walk_tree(data_node, &undefine_user_code);
}

sl_status_t get_all_pin_codes_command(dotdot_unid_t unid,
                                      dotdot_endpoint_id_t endpoint,
                                      uic_mqtt_dotdot_callback_call_type_t call_type)
{
  attribute_store_node_t user_code_data
    = get_user_code_data_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    return attribute_store_node_exists(user_code_data) ? SL_STATUS_OK
                                                       : SL_STATUS_FAIL;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  attribute_store_node_t data_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(DATA));

  uint16_t number_of_users = 0;
  attribute_store_get_child_reported(user_code_data,
                                     ATTRIBUTE(NUMBER_OF_USERS),
                                     &number_of_users,
                                     sizeof(number_of_users));

  for (uint16_t i = 1; (i <= number_of_users) && (i != 0); i++) {
    attribute_store_emplace(data_node, ATTRIBUTE(USER_ID), &i, sizeof(i));
  }

  undefine_all_user_codes(endpoint_node);

  return SL_STATUS_OK;
}

static sl_status_t
  set_user_status_command(dotdot_unid_t unid,
                          dotdot_endpoint_id_t endpoint,
                          uic_mqtt_dotdot_callback_call_type_t call_type,
                          DrlkTotalUserID userid,
                          DrlkSettableUserStatus user_status)
{
  attribute_store_node_t user_code_data
    = get_user_code_data_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    return attribute_store_node_exists(user_code_data) ? SL_STATUS_OK
                                                       : SL_STATUS_FAIL;
  }

  uint16_t user_id = userid;
  attribute_store_node_t user_id_node
    = attribute_store_get_node_child_by_value(user_code_data,
                                              ATTRIBUTE(USER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&user_id,
                                              sizeof(uint16_t),
                                              0);
  uint8_t user_id_status = user_status;
  if (user_id_status == ZCL_DRLK_USER_STATUS_OCCUPIED_DISABLED) {
    user_id_status = USER_STATUS_DISABLED;
  }
  return attribute_store_set_child_desired(user_id_node,
                                           ATTRIBUTE(USER_ID_STATUS),
                                           &user_id_status,
                                           sizeof(user_id_status));
}

static sl_status_t
  get_user_status_command(dotdot_unid_t unid,
                          dotdot_endpoint_id_t endpoint,
                          uic_mqtt_dotdot_callback_call_type_t call_type,
                          DrlkTotalUserID userid)
{
  attribute_store_node_t user_code_data
    = get_user_code_data_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    return attribute_store_node_exists(user_code_data) ? SL_STATUS_OK
                                                       : SL_STATUS_FAIL;
  }

  uint16_t user_id = userid;
  attribute_store_node_t user_id_node
    = attribute_store_get_node_child_by_value(user_code_data,
                                              ATTRIBUTE(USER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&user_id,
                                              sizeof(uint16_t),
                                              0);
  attribute_store_node_t user_id_status_node
    = attribute_store_get_first_child_by_type(user_id_node,
                                              ATTRIBUTE(USER_ID_STATUS));
  attribute_store_undefine_desired(user_id_status_node);
  attribute_store_undefine_reported(user_id_status_node);

  return attribute_store_node_exists(user_id_status_node) ? SL_STATUS_OK
                                                          : SL_STATUS_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute publication functions
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Attribute store callback functions
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//  Init and teardown functions.
//////////////////////////////////////////////////////////////////////////////
sl_status_t user_code_cluster_server_init()
{
  sl_log_debug(LOG_TAG, "User Code server initialization");
  // Register the callback for handling commands from IoT service
  uic_mqtt_dotdot_door_lock_setpin_code_callback_set(&set_pin_code_command);
  uic_mqtt_dotdot_door_lock_getpin_code_callback_set(&get_pin_code_command);
  uic_mqtt_dotdot_door_lock_get_all_pin_codes_callback_set(&get_all_pin_codes_command);
  uic_mqtt_dotdot_door_lock_clearpin_code_callback_set(&clear_pin_code_command);
  uic_mqtt_dotdot_door_lock_clear_allpin_codes_callback_set(
    &clear_all_pin_codes_command);
  uic_mqtt_dotdot_door_lock_set_user_status_callback_set(
    &set_user_status_command);
  uic_mqtt_dotdot_door_lock_get_user_status_callback_set(
    &get_user_status_command);

  return SL_STATUS_OK;
}
