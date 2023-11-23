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
#include "zwave_command_class_user_code.h"
#include "zwave_command_classes_utils.h"

// Interfaces
#include "ZW_classcmd.h"
#include "attribute_store_defined_attribute_types.h"

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_command_handler.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_utils.h"
#include "zpc_attribute_store_network_helper.h"
#include "zpc_attribute_resolver.h"
#include "zwave_command_class_user_code_types.h"

// Includes from Unify Components
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_generated_commands.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"

// Generic includes
#include <string.h>

#include "sl_log.h"
#define LOG_TAG "zwave_command_class_user_code"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_USER_CODE_##type

static const attribute_store_type_t default_attributes[]
  = {ATTRIBUTE(DELETE_ALL_REQUESTED),
     ATTRIBUTE(NUMBER_OF_USERS),
     ATTRIBUTE(CAPABILITIES)};

static const attribute_store_type_t v2_attributes[] = {ATTRIBUTE(KEYPAD_MODE)};
static const attribute_store_type_t user_id_attributes[]
  = {ATTRIBUTE(CODE), ATTRIBUTE(USER_ID_STATUS)};

static const attribute_store_type_t capabilities_attributes[]
  = {ATTRIBUTE(SUPPORTED_FLAGS),
     ATTRIBUTE(SUPPORTED_USER_ID_STATUS),
     ATTRIBUTE(SUPPORTED_KEYPAD_MODES),
     ATTRIBUTE(SUPPORTED_KEYS)};

// V1 nodes support codes 0 to 9, so ASCII 48 to 57.
// Mask 7 -> All bits
// Mask 8 -> bit 0 and bit 1.
#define KEY_BITMASK_SIZE 16
static const uint8_t V1_SUPPORTED_KEYS[KEY_BITMASK_SIZE] = {0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0xFF,
                                                            0x03,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00};

static attribute_store_node_t
  get_user_code_data_node(const zwave_controller_connection_info_t *info)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(info);
  return attribute_store_get_first_child_by_type(endpoint_node,
                                                 ATTRIBUTE(DATA));
}

static attribute_store_node_t
  get_user_code_capabilities_node(attribute_store_node_t endpoint_node)
{
  return attribute_store_get_first_child_by_type(
    attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(DATA)),
    ATTRIBUTE(CAPABILITIES));
}

static attribute_store_node_t get_user_id_node(attribute_store_node_t data_node,
                                               uint16_t user_id)
{
  return attribute_store_get_node_child_by_value(data_node,
                                                 ATTRIBUTE(USER_ID),
                                                 REPORTED_ATTRIBUTE,
                                                 (uint8_t *)&user_id,
                                                 sizeof(user_id),
                                                 0);
}

/**
 * @brief Checks if the user_id_node type is correct and sets the
 * user code data to Available underneath
 *
 * @param user_id_node  Attribute Store node for the User ID.
 */
static void set_user_code_to_available(attribute_store_node_t user_id_node)
{
  if (attribute_store_get_node_type(user_id_node) != ATTRIBUTE(USER_ID)) {
    return;
  }

  // Set the user code to nothing:
  attribute_store_node_t user_code_node
    = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  const char empty_string = '\0';
  attribute_store_set_reported(user_code_node,
                               &empty_string,
                               sizeof(empty_string));
  attribute_store_undefine_desired(user_code_node);

  attribute_store_node_t user_id_status_node
    = attribute_store_get_first_child_by_type(user_id_node,
                                              ATTRIBUTE(USER_ID_STATUS));
  const uint8_t user_id_status = 0;
  attribute_store_set_reported(user_id_status_node,
                               &user_id_status,
                               sizeof(user_id_status));
  attribute_store_undefine_desired(user_id_status_node);
}

/**
 * @brief Sets all users Id in the Range [first_id..last_id[ to available (user code not set)
 *
 * @param user_id_node  Attribute Store node for the User ID.
 */
static void set_user_code_range_to_available(attribute_store_node_t data_node,
                                             uint16_t first_id,
                                             uint16_t last_id)
{
  if (first_id >= last_id) {
    return;
  }
  for (uint16_t i = first_id; i < last_id; i++) {
    set_user_code_to_available(get_user_id_node(data_node, i));
  }
}

/**
 * @brief Navigates up the tree from a node and makes sure that all user codes are
 * set to "disabled" with no pin code
 *
 * @param node  Attribute Store node located under the  ATTRIBUTE(DATA)
 */
static void set_all_user_codes_to_available(attribute_store_node_t node)
{
  attribute_store_node_t data_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(DATA));

  attribute_store_walk_tree(data_node, &set_user_code_to_available);
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

/**
 * @brief Sets all the default capabilities for version 1 supporting nodes.
 *
 * @param node  Attribute Store ATTRIBUTE(CAPABILITIES) node
 */
static void set_default_v1_capabilities(attribute_store_node_t node)
{
  const uint8_t V1_SUPPORTED_FLAGS = 0x0;
  attribute_store_set_child_reported(node,
                                     ATTRIBUTE(SUPPORTED_FLAGS),
                                     &V1_SUPPORTED_FLAGS,
                                     sizeof(V1_SUPPORTED_FLAGS));

  const uint8_t V1_SUPPORTED_USER_ID_STATUS_BITMASK = 0x7;
  attribute_store_set_child_reported(
    node,
    ATTRIBUTE(SUPPORTED_USER_ID_STATUS),
    &V1_SUPPORTED_USER_ID_STATUS_BITMASK,
    sizeof(V1_SUPPORTED_USER_ID_STATUS_BITMASK));

  const uint8_t V1_SUPPORTED_KEYPAD_MODES_BITMASK = 0x1;
  attribute_store_set_child_reported(node,
                                     ATTRIBUTE(SUPPORTED_KEYPAD_MODES),
                                     &V1_SUPPORTED_KEYPAD_MODES_BITMASK,
                                     sizeof(V1_SUPPORTED_KEYPAD_MODES_BITMASK));

  // V1 supported keys
  attribute_store_set_child_reported(node,
                                     ATTRIBUTE(SUPPORTED_KEYS),
                                     &V1_SUPPORTED_KEYS,
                                     sizeof(V1_SUPPORTED_KEYS));
}

/**
 * @brief Checks the nodes capabilities and verifies if a character is supported
 * for setting in a PIN code.
 *
 * NOTE: This function assumes to receive a NULL terminated PIN Code.
 *
 * @param endpoint_node  Attribute Store node for the Endpoint
 * @param pin_code       PIN Code to check all chars
 */
static bool is_pin_code_supported(attribute_store_node_t endpoint_node,
                                  const char *pin_code)
{
  attribute_store_node_t capabilities_node
    = get_user_code_capabilities_node(endpoint_node);

  attribute_store_node_t supported_keys_node
    = attribute_store_get_first_child_by_type(capabilities_node,
                                              ATTRIBUTE(SUPPORTED_KEYS));
  if (false == attribute_store_is_reported_defined(supported_keys_node)) {
    sl_log_debug(LOG_TAG,
                 "Capabilities are unkonwn, assuming pin code %s "
                 "is supported for endpoint node %d.",
                 pin_code,
                 endpoint_node);
    return true;
  }

  uint8_t supported_keys[KEY_BITMASK_SIZE] = {};
  attribute_store_get_reported(
    supported_keys_node,
    supported_keys,
    attribute_store_get_reported_size(supported_keys_node));

  for (int i = 0; i < strlen(pin_code); i++) {
    char c       = pin_code[i];
    uint8_t byte = c >> 3;          // Divide by 8
    uint8_t bit  = 1 << (c & 0x7);  // modulo 8

    if ((supported_keys[byte] & bit) == 0) {
      sl_log_warning(LOG_TAG,
                     "Char %c is not supported: (bitmask %d value %02X)",
                     c,
                     byte,
                     supported_keys[byte]);
      return false;
    }
  }
  return true;
}

/**
 * @brief Checks the nodes capabilities and verifies if it
 * supports MUCR (Multiple User Code Reports)
 *
 * @param endpoint_node  Attribute Store node for the Endpoint
 */
static bool is_mucr_supported(attribute_store_node_t endpoint_node)
{
  attribute_store_node_t capabilities_node
    = get_user_code_capabilities_node(endpoint_node);
  int8_t supported_flags = 0x0;
  attribute_store_get_child_reported(capabilities_node,
                                     ATTRIBUTE(SUPPORTED_FLAGS),
                                     &supported_flags,
                                     sizeof(supported_flags));

  return ((1 << 4) & supported_flags);
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Generates a Users Number Get Command
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_user_code_number_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  // This is a static payload.
  ZW_USERS_NUMBER_GET_V2_FRAME *get_frame
    = (ZW_USERS_NUMBER_GET_V2_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_USER_CODE_V2;
  get_frame->cmd      = USERS_NUMBER_GET_V2;
  *frame_length       = sizeof(ZW_USERS_NUMBER_GET_V2_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a (opt. Extended) User Code Get Command
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_user_code_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t version_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(VERSION));
  // We need to check the version of the supporting node:
  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait to know the supporting node version
    *frame_length = 0;
    return SL_STATUS_IS_WAITING;
  }

  attribute_store_node_t user_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(USER_ID));

  uint16_t user_id = 0;
  attribute_store_get_reported(user_id_node, &user_id, sizeof(user_id));

  if (supporting_node_version == 1) {
    if (user_id > 255) {
      sl_log_warning(LOG_TAG, "Invalid User ID detected. Deleting");
      attribute_store_delete_node(user_id_node);
      *frame_length = 0;
      return SL_STATUS_ALREADY_EXISTS;
    }
    frame[0]      = COMMAND_CLASS_USER_CODE_V2;
    frame[1]      = USER_CODE_GET_V2;
    frame[2]      = (uint8_t)user_id;
    *frame_length = 3;
    return SL_STATUS_OK;
  }

  ZW_EXTENDED_USER_CODE_GET_V2_FRAME *get_frame
    = (ZW_EXTENDED_USER_CODE_GET_V2_FRAME *)frame;
  get_frame->cmdClass        = COMMAND_CLASS_USER_CODE_V2;
  get_frame->cmd             = EXTENDED_USER_CODE_GET_V2;
  get_frame->userIdentifier1 = user_id >> 8;
  get_frame->userIdentifier2 = user_id & 0xFF;
  get_frame->properties1     = is_mucr_supported(endpoint_node) ? 1 : 0;
  *frame_length              = sizeof(ZW_EXTENDED_USER_CODE_GET_V2_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a (opt. Extended) User Code Get Command
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_user_code_capabilities_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t version_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(VERSION));
  // We need to check the version of the supporting node:
  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait to know the supporting node version
    *frame_length = 0;
    return SL_STATUS_IS_WAITING;
  }

  if (supporting_node_version == 1) {
    attribute_store_node_t capabilities_node
      = attribute_store_get_first_parent_with_type(node,
                                                   ATTRIBUTE(CAPABILITIES));
    set_default_v1_capabilities(capabilities_node);
    return SL_STATUS_ALREADY_EXISTS;
  }

  ZW_USER_CODE_CAPABILITIES_GET_V2_FRAME *get_frame
    = (ZW_USER_CODE_CAPABILITIES_GET_V2_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_USER_CODE_V2;
  get_frame->cmd      = USER_CODE_CAPABILITIES_GET_V2;
  *frame_length       = sizeof(ZW_USER_CODE_CAPABILITIES_GET_V2_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a User Code Checksum Get Command
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_user_code_checksum_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t version_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(VERSION));
  // We need to check the version of the supporting node:
  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait to know the supporting node version
    *frame_length = 0;
    return SL_STATUS_IS_WAITING;
  }

  if (supporting_node_version == 1) {
    sl_log_warning(LOG_TAG, "Checksum attribute found for v1 node. Deleting");
    attribute_store_delete_node(node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  ZW_USER_CODE_CHECKSUM_GET_V2_FRAME *get_frame
    = (ZW_USER_CODE_CHECKSUM_GET_V2_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_USER_CODE_V2;
  get_frame->cmd      = USER_CODE_CHECKSUM_GET_V2;
  *frame_length       = sizeof(ZW_USER_CODE_CHECKSUM_GET_V2_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates an Admin Code Get Command
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_user_code_admin_code_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t version_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(VERSION));
  // We need to check the version of the supporting node:
  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait to know the supporting node version
    *frame_length = 0;
    return SL_STATUS_IS_WAITING;
  }

  if (supporting_node_version == 1) {
    sl_log_warning(LOG_TAG, "Admin Code attribute found for v1 node. Deleting");
    attribute_store_delete_node(node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  zwave_minimum_frame_t *get_frame = (zwave_minimum_frame_t *)frame;
  get_frame->command_class         = COMMAND_CLASS_USER_CODE_V2;
  get_frame->command               = 0x0F;
  *frame_length                    = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Keypad Mode Get Command
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_user_code_keypad_mode_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  zwave_minimum_frame_t *get_frame = (zwave_minimum_frame_t *)frame;
  get_frame->command_class         = COMMAND_CLASS_USER_CODE_V2;
  get_frame->command               = USER_CODE_KEYPAD_MODE_GET_V2;
  *frame_length                    = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Keypad Mode Set Command
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_user_code_keypad_mode_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  ZW_USER_CODE_KEYPAD_MODE_SET_V2_FRAME *set_frame
    = (ZW_USER_CODE_KEYPAD_MODE_SET_V2_FRAME *)frame;
  set_frame->cmdClass = COMMAND_CLASS_USER_CODE_V2;
  set_frame->cmd      = USER_CODE_KEYPAD_MODE_SET_V2;
  attribute_store_get_desired_else_reported(node,
                                            &set_frame->keypadMode,
                                            sizeof(uint8_t));
  *frame_length = sizeof(ZW_USER_CODE_KEYPAD_MODE_SET_V2_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a (opt. Extended) User Code Set Command
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_user_code_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t version_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(VERSION));
  // We need to check the version of the supporting node:
  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait to know the supporting node version
    *frame_length = 0;
    return SL_STATUS_IS_WAITING;
  }

  attribute_store_node_t user_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(USER_ID));
  uint16_t user_id = 0;
  attribute_store_get_reported(user_id_node, &user_id, sizeof(user_id));
  attribute_store_node_t user_id_status_node
    = attribute_store_get_first_child_by_type(user_id_node,
                                              ATTRIBUTE(USER_ID_STATUS));
  attribute_store_node_t user_code_node
    = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));

  char pin_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  // Safety: NULL termination of pin_code is guaranteed by the following function.
  attribute_store_get_desired_else_reported_string(
    user_code_node,
    pin_code,
    ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  if (false == is_pin_code_supported(endpoint_node, pin_code)) {
    sl_log_warning(LOG_TAG,
                   "Invalid character in Desired User Code value %s for "
                   "Endpoint ID Node %d. Undefining desired values.",
                   pin_code,
                   endpoint_node);
    attribute_store_undefine_desired(user_code_node);
    attribute_store_undefine_desired(user_id_status_node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  if (supporting_node_version == 1) {
    if (user_id > 255) {
      sl_log_warning(LOG_TAG, "Invalid User ID detected. Deleting");
      attribute_store_delete_node(user_id_node);
      *frame_length = 0;
      return SL_STATUS_ALREADY_EXISTS;
    }
    uint8_t user_code_size
      = (attribute_store_get_desired_else_reported_size(user_code_node) - 1)
        & 0x0F;
    if (user_code_size < 4) {
      sl_log_warning(LOG_TAG,
                     "Invalid User Code length detected. "
                     "Undefining desired values");
      attribute_store_undefine_desired(user_code_node);
      attribute_store_undefine_desired(user_id_status_node);
      *frame_length = 0;
      return SL_STATUS_ALREADY_EXISTS;
    }

    uint8_t user_id_status = 0;
    attribute_store_get_desired_else_reported(user_id_status_node,
                                              &user_id_status,
                                              sizeof(user_id_status));

    frame[0] = COMMAND_CLASS_USER_CODE_V2;
    frame[1] = USER_CODE_SET_V2;
    frame[2] = (uint8_t)user_id;
    frame[3] = user_id_status;
    if (user_id_status == 0) {
      user_code_size = 4;
      frame[4]       = 0x00;
      frame[5]       = 0x00;
      frame[6]       = 0x00;
      frame[7]       = 0x00;
    } else {
      attribute_store_get_desired_else_reported_string(user_code_node,
                                                       (char *)&frame[4],
                                                       MAX_FRAME_LEN - 5);
    }

    *frame_length = 4 + user_code_size;
    attribute_store_undefine_desired(user_code_node);
    attribute_store_undefine_reported(user_code_node);
    return SL_STATUS_OK;
  }

  uint8_t user_id_status = 0;
  attribute_store_get_desired_else_reported(user_id_status_node,
                                            &user_id_status,
                                            sizeof(user_id_status));

  frame[0] = COMMAND_CLASS_USER_CODE_V2;
  frame[1] = EXTENDED_USER_CODE_SET_V2;
  frame[2] = 1;                 // number of user codes
  frame[3] = user_id >> 8;      // User ID MSB
  frame[4] = (user_id & 0xFF);  // User ID LSB
  frame[5] = user_id_status;

  uint8_t user_code_size = 0;
  if (user_id_status != 0) {
    user_code_size
      = (attribute_store_get_desired_else_reported_size(user_code_node) - 1)
        & 0x0F;
    attribute_store_get_desired_else_reported_string(user_code_node,
                                                     (char *)&frame[7],
                                                     MAX_FRAME_LEN - 7);
  }

  frame[6]      = user_code_size;
  *frame_length = 7 + user_code_size;
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Admin Code Set Command
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_user_code_admin_code_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Check that the Admin code can be used with that supporting node.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  char admin_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  // Safety: NULL termination of admin_code is guaranteed by the following function
  attribute_store_get_desired_else_reported_string(
    node,
    admin_code,
    ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);

  if (false == is_pin_code_supported(endpoint_node, admin_code)) {
    sl_log_warning(LOG_TAG,
                   "Invalid character in Desired Admin Code value %s for "
                   "Endpoint ID Node %d. Undefining desired value.",
                   admin_code,
                   endpoint_node);
    attribute_store_undefine_desired(node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  frame[0] = COMMAND_CLASS_USER_CODE_V2;
  frame[1] = 0x0E;  // ADMIN_CODE_SET_V2;

  uint8_t admin_code_size
    = attribute_store_get_desired_else_reported_size(node) - 1;
  if (admin_code_size > 16) {
    sl_log_debug(LOG_TAG, "Code length longer than 16 bytes. Clipping to 16.");
    admin_code_size = 16;
  }
  frame[2] = admin_code_size;
  attribute_store_get_desired_else_reported_string(node,
                                                   (char *)&frame[3],
                                                   MAX_FRAME_LEN - 3);

  *frame_length = 3 + admin_code_size;
  return SL_STATUS_OK;
}

/**
 * @brief Generates a User Code Set or Extended User Code Set that will delete
 * all User Codes.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_user_code_delete_all(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t version_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(VERSION));
  // We need to check the version of the supporting node:
  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    sl_log_debug(LOG_TAG,
                 "Unknown supporting node version, "
                 "waiting before deleting all user codes.");
    *frame_length = 0;
    return SL_STATUS_IS_WAITING;
  }

  if (supporting_node_version == 1) {
    ZW_USER_CODE_SET_4BYTE_FRAME *set_frame
      = (ZW_USER_CODE_SET_4BYTE_FRAME *)frame;
    set_frame->cmdClass       = COMMAND_CLASS_USER_CODE_V2;
    set_frame->cmd            = USER_CODE_SET_V2;
    set_frame->userIdentifier = 0;
    set_frame->userIdStatus   = 0;
    set_frame->userCode1      = 0;
    set_frame->userCode2      = 0;
    set_frame->userCode3      = 0;
    set_frame->userCode4      = 0;
    *frame_length             = sizeof(ZW_USER_CODE_SET_4BYTE_FRAME);
    return SL_STATUS_OK;
  }

  // We can send a V2 extended Set:
  ZW_EXTENDED_USER_CODE_SET_1BYTE_V2_FRAME *set_frame
    = (ZW_EXTENDED_USER_CODE_SET_1BYTE_V2_FRAME *)frame;
  set_frame->cmdClass                      = COMMAND_CLASS_USER_CODE_V2;
  set_frame->cmd                           = EXTENDED_USER_CODE_SET_V2;
  set_frame->numberOfUserCodes             = 1;
  set_frame->variantgroup1.userIdentifier1 = 0;
  set_frame->variantgroup1.userIdentifier2 = 0;
  set_frame->variantgroup1.userIdStatus    = 0;
  set_frame->variantgroup1.properties1     = 0;
  *frame_length                            = 7;
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions
///////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_user_code_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_USER_CODE_V2,
        updated_node)
      == true) {
    return;
  }

  // Do not create the attributes until we are sure of the version
  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(updated_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait that the version becomes non-zero.
    return;
  }

  // Now we know we have a Multilevel Switch supporting endpoint.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t data_node
    = attribute_store_create_child_if_missing(endpoint_node, ATTRIBUTE(DATA));

  attribute_store_add_if_missing(data_node,
                                 default_attributes,
                                 COUNT_OF(default_attributes));
  const command_status_values_t status = FINAL_STATE;
  attribute_store_set_child_reported(data_node,
                                     ATTRIBUTE(DELETE_ALL_REQUESTED),
                                     &status,
                                     sizeof(status));

  // Create the v2 attributes if needed
  if (supporting_node_version >= 2) {
    attribute_store_add_if_missing(data_node,
                                   v2_attributes,
                                   COUNT_OF(v2_attributes));
  }
}

static void zwave_command_class_user_code_on_number_of_users_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }
  uint16_t number_of_users = 0;
  attribute_store_get_reported(updated_node,
                               &number_of_users,
                               sizeof(number_of_users));

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t version_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(VERSION));
  // We need to check the version of the supporting node:
  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));
  if (supporting_node_version == 1) {
    sl_log_debug(LOG_TAG, "Skip creating User ID nodes for CC UserCode V1.");
    return;
  }

  attribute_store_node_t data_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(DATA));

  for (uint16_t i = 1; (i <= number_of_users) && (i != 0); i++) {
    attribute_store_emplace(data_node, ATTRIBUTE(USER_ID), &i, sizeof(i));
  }
}

static void zwave_command_class_user_code_on_user_id_created(
  attribute_store_node_t node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_CREATED) {
    return;
  }
  attribute_store_add_if_missing(node,
                                 user_id_attributes,
                                 COUNT_OF(user_id_attributes));
}

static void zwave_command_class_user_code_on_capabilities_created(
  attribute_store_node_t node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_CREATED) {
    return;
  }
  attribute_store_add_if_missing(node,
                                 capabilities_attributes,
                                 COUNT_OF(capabilities_attributes));
}

static void zwave_command_class_user_code_on_code_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  // Get pin code value
  char pin_code [ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {0};
  attribute_store_get_reported_string(updated_node,
                                      pin_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  attribute_store_node_t user_id_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE(USER_ID));
  // get user id value
  uint16_t user_id = 0;
  attribute_store_get_reported(user_id_node, &user_id, sizeof(user_id));

  attribute_store_node_t endpoint_id_node
    = attribute_store_get_first_parent_with_type(user_id_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_get_reported(endpoint_id_node, &endpoint_id, sizeof(endpoint_id));

  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(endpoint_id_node,
                                                 ATTRIBUTE_NODE_ID);
  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));

  unid_t node_unid;
  zwave_unid_from_node_id(node_id, node_unid);

  attribute_store_node_t status_node
    = attribute_store_get_first_child_by_type(user_id_node,
                                              ATTRIBUTE(USER_ID_STATUS));
  uint8_t ustatus = 0;
  attribute_store_get_reported(status_node, &ustatus, sizeof(ustatus));
  DrlkUserStatus user_status = 0;
  switch (ustatus) {
    case USER_STATUS_AVAILABLE:
      user_status = ZCL_DRLK_USER_STATUS_AVAILABLE;
      break;
    case USER_STATUS_ENABLED:
      user_status = ZCL_DRLK_USER_STATUS_OCCUPIED_ENABLED;
      break;
    case USER_STATUS_DISABLED:
      user_status = ZCL_DRLK_USER_STATUS_OCCUPIED_DISABLED;
      break;
    case USER_STATUS_MESSAGING:
      user_status = ZCL_DRLK_USER_STATUS_OCCUPIED_ENABLED;
      break;
    case USER_STATUS_PASSAGE_MODE:
      user_status = ZCL_DRLK_USER_STATUS_OCCUPIED_DISABLED;
      break;
  }

  if (attribute_store_is_desired_defined(updated_node) == false) {
    uic_mqtt_dotdot_door_lock_command_get_pin_code_response_fields_t fields =
      {
        .userid = (DrlkPINUserID) user_id,
        .user_status = (DrlkUserStatus) user_status,
        .user_type = (DrlkUserType) ZCL_DRLK_USER_TYPE_UNRESTRICTED_USER,
        .code = pin_code
      };

    uic_mqtt_dotdot_door_lock_publish_generated_get_pin_code_response_command(
      node_unid, endpoint_id, &fields);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver callback functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Handles the Send Data Complete for a "delete all user codes"
 *
 * The resolver alone does not support this case, because the attribute being
 * resolved and the attributes affected are located in different places.
 *
 * This function manually traverses the tree and adjust the value of
 * User ID statuses / User Codes based on the outcome of the Delete All Command
 *
 * @param node           Attribute Store node that was resolved
 * @param rule_type      Type of rule that was applied to the node
 * @param event          Resolution outcome/event
 */
static void on_delete_all_send_data_complete(attribute_store_node_t node,
                                             resolver_rule_type_t rule_type,
                                             zpc_resolver_event_t event)
{
  command_status_values_t state = FINAL_STATE;
  if (rule_type == RESOLVER_GET_RULE) {
    sl_log_warning(LOG_TAG,
                   "Delete All Codes Requested attribute "
                   "should not be 'Get Resolved'");
    attribute_store_set_desired(node, &state, sizeof(state));
    attribute_store_set_reported(node, &state, sizeof(state));
    return;
  }

  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      // Wait for more. Weird to have a working Set Default.
      return;

    case FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT:
      // This is strange. Log a warning here as this should not happen.
      sl_log_warning(LOG_TAG,
                     "User Code supporting node reports that erasing all "
                     "User codes is not supported.",
                     node);
      break;
    case FRAME_SENT_EVENT_OK_SUPERVISION_FAIL:
      sl_log_debug(LOG_TAG,
                   "Erasing all user code failed. "
                   "User Code database will be read again.");
      undefine_all_user_codes(attribute_store_get_first_parent_with_type(node,ATTRIBUTE_ENDPOINT_ID));
      break;

    //FRAME_SENT_EVENT_OK_NO_SUPERVISION
    //FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS
    default:
      set_all_user_codes_to_available(node);
      break;
  }

  // Do not try to resolve this again: (only 1 set default command)
  attribute_store_set_desired(node, &state, sizeof(state));
  attribute_store_set_reported(node, &state, sizeof(state));
}

///////////////////////////////////////////////////////////////////////////////
// Command Class frame handler functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Process an incoming User Number Report Command.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t
  handle_users_number_report(const zwave_controller_connection_info_t *info,
                             const uint8_t *frame,
                             uint16_t frame_length)
{
  uint16_t number_of_users = 0;
  if (frame_length >= 5) {
    number_of_users = (uint16_t)(frame[3] << 8 | frame[4]);
  } else if (frame_length >= 3) {
    number_of_users = frame[2];
  }

  attribute_store_node_t data_node = get_user_code_data_node(info);
  attribute_store_set_child_reported(data_node,
                                     ATTRIBUTE(NUMBER_OF_USERS),
                                     &number_of_users,
                                     sizeof(number_of_users));
  return SL_STATUS_OK;
}

/**
 * @brief Process an incoming User Code Checksum Report Command.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t handle_users_code_checksum_report(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  uint16_t checksum = 0;
  if (frame_length >= 4) {
    checksum = (uint16_t)(frame[2] << 8 | frame[3]);
  }

  attribute_store_node_t data_node = get_user_code_data_node(info);
  attribute_store_set_child_reported(data_node,
                                     ATTRIBUTE(CHECKSUM),
                                     &checksum,
                                     sizeof(checksum));
  return SL_STATUS_OK;
}

/**
 * @brief Process an incoming User Code Report Command.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t
  handle_user_code_report(const zwave_controller_connection_info_t *info,
                          const uint8_t *frame,
                          uint16_t frame_length)
{
  if (frame_length < 4) {
    sl_log_debug(LOG_TAG, "User Report frame too short. Discarding");
    return SL_STATUS_OK;
  }

  uint16_t user_id                 = frame[2];
  uint8_t user_id_status           = frame[3];
  attribute_store_node_t data_node = get_user_code_data_node(info);
  attribute_store_node_t user_id_node = get_user_id_node(data_node, user_id);

  // Save the user ID Status
  attribute_store_node_t user_id_status_node
    = attribute_store_get_first_child_by_type(user_id_node,
                                              ATTRIBUTE(USER_ID_STATUS));
  attribute_store_set_reported(user_id_status_node,
                               &user_id_status,
                               sizeof(user_id_status));
  attribute_store_undefine_desired(user_id_status_node);

  // Save the user code. Add a NULL termination.
  uint8_t user_code_length = frame_length - 4;
  attribute_store_node_t user_code_node
    = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));

  char user_code[user_code_length + 1];
  memcpy(user_code, &frame[4], user_code_length);
  user_code[user_code_length] = '\0';
  attribute_store_set_reported_string(user_code_node, user_code);
  if (user_code_node != ATTRIBUTE_STORE_INVALID_NODE) {
    zwave_command_class_user_code_on_code_update(user_code_node,
                                                 ATTRIBUTE_UPDATED);
  }
  attribute_store_undefine_desired(user_code_node);

  return SL_STATUS_OK;
}

/**
 * @brief Process an incoming Admin Code Report Command.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t
  handle_admin_code_report(const zwave_controller_connection_info_t *info,
                           const uint8_t *frame,
                           uint16_t frame_length)
{
  if (frame_length < 3) {
    sl_log_debug(LOG_TAG, "Admin Code Report frame too short. Discarding");
    return SL_STATUS_OK;
  }

  attribute_store_node_t data_node = get_user_code_data_node(info);

  uint8_t admin_code_length = frame[2] & 0x0F;
  char admin_code[admin_code_length + 1];
  admin_code[0] = '\0';

  attribute_store_node_t admin_code_node
    = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(ADMIN_CODE));
  if (3 + admin_code_length <= frame_length) {
    memcpy(admin_code, &frame[3], admin_code_length);
    admin_code[admin_code_length] = '\0';
  }

  attribute_store_set_reported_string(admin_code_node, admin_code);
  attribute_store_undefine_desired(admin_code_node);

  return SL_STATUS_OK;
}

/**
 * @brief Process an incoming Extended User Code Report Command.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t handle_extended_user_code_report(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length < 7) {
    sl_log_debug(LOG_TAG, "Extended User Report frame too short. Discarding");
    return SL_STATUS_OK;
  }

  attribute_store_node_t data_node = get_user_code_data_node(info);

  uint8_t number_of_user_codes = frame[2];
  uint16_t last_user_id        = 0;
  uint16_t j                   = 3;
  for (uint8_t i = 0; i < number_of_user_codes; i++) {
    if (j + 4 > frame_length) {
      break;
    }
    uint16_t user_id         = (uint16_t)(frame[j] << 8 | frame[j + 1]);
    uint8_t user_id_status   = frame[j + 2];
    uint8_t user_code_length = frame[j + 3] & 0x0F;

    if (last_user_id != 0) {
      set_user_code_range_to_available(data_node, last_user_id + 1, user_id);
    }
    last_user_id = user_id;

    attribute_store_node_t user_id_node = get_user_id_node(data_node, user_id);
    attribute_store_node_t user_id_status_node
      = attribute_store_get_first_child_by_type(user_id_node,
                                                ATTRIBUTE(USER_ID_STATUS));
    attribute_store_set_reported(user_id_status_node,
                                 &user_id_status,
                                 sizeof(user_id_status));
    attribute_store_undefine_desired(user_id_status_node);

    attribute_store_node_t user_code_node
      = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));

    char user_code[user_code_length + 1];
    user_code[0] = '\0';
    if (j + 4 + user_code_length <= frame_length) {
      memcpy(user_code, &frame[j + 4], user_code_length);
      user_code[user_code_length] = '\0';
    }

    attribute_store_set_reported_string(user_code_node, user_code);
    if (user_code_node != ATTRIBUTE_STORE_INVALID_NODE) {
      zwave_command_class_user_code_on_code_update(user_code_node,
                                                 ATTRIBUTE_UPDATED);
    }
    attribute_store_undefine_desired(user_code_node);

    j += 4 + user_code_length;
  }

  // Next user code is the last 2 bytes
  uint16_t next_user_id
    = (uint16_t)((frame[frame_length - 2] << 8) | frame[frame_length - 1]);
  if (next_user_id == 0) {
    // If this field is set to 0, it means no more user codes are set.
    // Set this to the last User ID:
    attribute_store_get_child_reported(data_node,
                                       ATTRIBUTE(NUMBER_OF_USERS),
                                       &next_user_id,
                                       sizeof(next_user_id));
  }
  if (last_user_id != 0) {
    set_user_code_range_to_available(data_node, last_user_id + 1, next_user_id);
  }

  return SL_STATUS_OK;
}

/**
 * @brief Process an incoming User Code Capabilities Report Command.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t handle_user_code_capabilities_report(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  attribute_store_node_t data_node = get_user_code_data_node(info);
  attribute_store_node_t capabilities_node
    = attribute_store_get_first_child_by_type(data_node,
                                              ATTRIBUTE(CAPABILITIES));

  if (frame_length < 5) {
    set_default_v1_capabilities(capabilities_node);
    sl_log_debug(LOG_TAG,
                 "User Code Capabilities Report frame too short. Discarding. "
                 "V1 capabilities will be assumed.");
    return SL_STATUS_OK;
  }

  uint8_t i     = 2;
  uint8_t flags = 0;
  flags |= ((frame[i] & 0x80) == 0x80) ? 1 : 0;
  flags |= ((frame[i] & 0x40) == 0x40) ? (1 << 1) : 0;
  flags |= ((frame[i] & 0x20) == 0x20) ? (1 << 2) : 0;

  uint8_t user_id_status_length = frame[i] & 0x1F;
  if (user_id_status_length > 0
      && (frame_length >= (i + 1 + user_id_status_length))) {
    attribute_store_set_child_reported(capabilities_node,
                                       ATTRIBUTE(SUPPORTED_USER_ID_STATUS),
                                       &frame[i + 1],
                                       user_id_status_length);
  } else {
    const uint8_t V1_SUPPORTED_USER_ID_STATUS_BITMASK = 0x7;
    attribute_store_set_child_reported(
      capabilities_node,
      ATTRIBUTE(SUPPORTED_USER_ID_STATUS),
      &V1_SUPPORTED_USER_ID_STATUS_BITMASK,
      sizeof(V1_SUPPORTED_USER_ID_STATUS_BITMASK));
  }
  i += 1 + user_id_status_length;

  if (frame_length > i) {
    flags |= ((frame[i] & 0x80) == 0x80) ? (1 << 3) : 0;
    flags |= ((frame[i] & 0x40) == 0x40) ? (1 << 4) : 0;
    flags |= ((frame[i] & 0x20) == 0x20) ? (1 << 5) : 0;
  }
  attribute_store_set_child_reported(capabilities_node,
                                     ATTRIBUTE(SUPPORTED_FLAGS),
                                     &flags,
                                     sizeof(flags));

  uint8_t keypad_mode_length = 0;
  if (frame_length > i) {
    keypad_mode_length = frame[i] & 0x1F;
  }
  if ((keypad_mode_length > 0)
      && (frame_length >= (i + 1 + keypad_mode_length))) {
    attribute_store_set_child_reported(capabilities_node,
                                       ATTRIBUTE(SUPPORTED_KEYPAD_MODES),
                                       &frame[i + 1],
                                       keypad_mode_length);
  } else {
    const uint8_t V1_SUPPORTED_KEYPAD_MODES_BITMASK = 0x01;
    attribute_store_set_child_reported(
      capabilities_node,
      ATTRIBUTE(SUPPORTED_KEYPAD_MODES),
      &V1_SUPPORTED_KEYPAD_MODES_BITMASK,
      sizeof(V1_SUPPORTED_KEYPAD_MODES_BITMASK));
  }

  i += 1 + keypad_mode_length;
  uint8_t supported_keys_length = 0;
  if (frame_length > i) {
    supported_keys_length = frame[i] & 0x1F;
  }
  if ((supported_keys_length > 0)
      && (frame_length >= (i + 1 + supported_keys_length))) {
    attribute_store_set_child_reported(capabilities_node,
                                       ATTRIBUTE(SUPPORTED_KEYS),
                                       &frame[i + 1],
                                       supported_keys_length);
  } else {
    // V1 supported keys
    attribute_store_set_child_reported(capabilities_node,
                                       ATTRIBUTE(SUPPORTED_KEYS),
                                       &V1_SUPPORTED_KEYS,
                                       sizeof(V1_SUPPORTED_KEYS));
  }

  // Process Checksum / Admin code support flags:
  if (flags & 0x08) {
    sl_log_debug(LOG_TAG, "User Code Checksum supported. Creating attribute.");
    attribute_store_create_child_if_missing(data_node, ATTRIBUTE(CHECKSUM));
  }
  if (flags & 0x01) {
    sl_log_debug(LOG_TAG, "Admin Code supported. Creating attribute.");
    attribute_store_create_child_if_missing(data_node, ATTRIBUTE(ADMIN_CODE));
  }

  return SL_STATUS_OK;
}

/**
 * @brief Process an incoming Keypad Mode Report Command.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t
  handle_keypad_mode_report(const zwave_controller_connection_info_t *info,
                            const uint8_t *frame,
                            uint16_t frame_length)
{
  if (frame_length < 3) {
    sl_log_debug(LOG_TAG, "Keypad Mode Report frame too short. Discarding. ");
    return SL_STATUS_OK;
  }
  attribute_store_node_t data_node = get_user_code_data_node(info);
  attribute_store_node_t keypad_mode_node
    = attribute_store_get_first_child_by_type(data_node,
                                              ATTRIBUTE(KEYPAD_MODE));

  attribute_store_undefine_desired(keypad_mode_node);
  attribute_store_set_reported(keypad_mode_node, &frame[2], sizeof(uint8_t));
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Command handler functions
///////////////////////////////////////////////////////////////////////////////
/** @brief Dispatches incoming User Code Commands to individual
 *        command handlers.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_user_code_control_handler(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length < COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame[COMMAND_CLASS_INDEX] != COMMAND_CLASS_USER_CODE_V2) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame[COMMAND_INDEX]) {
    case USER_CODE_REPORT_V2:
      return handle_user_code_report(info, frame, frame_length);
    case EXTENDED_USER_CODE_REPORT_V2:
      return handle_extended_user_code_report(info, frame, frame_length);
    case USERS_NUMBER_REPORT_V2:
      return handle_users_number_report(info, frame, frame_length);
    case USER_CODE_CAPABILITIES_REPORT_V2:
      return handle_user_code_capabilities_report(info, frame, frame_length);
    case USER_CODE_KEYPAD_MODE_REPORT_V2:
      return handle_keypad_mode_report(info, frame, frame_length);
    case 0x10:  //ADMIN_CODE_REPORT_V2
      return handle_admin_code_report(info, frame, frame_length);
    case USER_CODE_CHECKSUM_REPORT_V2:
      return handle_users_code_checksum_report(info, frame, frame_length);

    default:
      sl_log_debug(LOG_TAG, "Just received a command of the future. Ignoring");
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_user_code_init()
{
  // Attribute store callbacks
  attribute_store_register_callback_by_type(
    &zwave_command_class_user_code_on_version_attribute_update,
    ATTRIBUTE(VERSION));
  attribute_store_register_callback_by_type(
    &zwave_command_class_user_code_on_number_of_users_update,
    ATTRIBUTE(NUMBER_OF_USERS));
  attribute_store_register_callback_by_type(
    &zwave_command_class_user_code_on_user_id_created,
    ATTRIBUTE(USER_ID));
  attribute_store_register_callback_by_type(
    &zwave_command_class_user_code_on_capabilities_created,
    ATTRIBUTE(CAPABILITIES));

  // Attribute resolver rules
  attribute_resolver_register_rule(ATTRIBUTE(NUMBER_OF_USERS),
                                   NULL,
                                   &zwave_command_class_user_code_number_get);

  attribute_resolver_register_rule(ATTRIBUTE(DELETE_ALL_REQUESTED),
                                   &zwave_command_class_user_code_delete_all,
                                   NULL);
  attribute_resolver_register_rule(ATTRIBUTE(CODE),
                                   &zwave_command_class_user_code_set,
                                   &zwave_command_class_user_code_get);
  attribute_resolver_register_rule(ATTRIBUTE(USER_ID_STATUS),
                                   &zwave_command_class_user_code_set,
                                   &zwave_command_class_user_code_get);

  attribute_resolver_register_rule(ATTRIBUTE(CHECKSUM),
                                   NULL,
                                   &zwave_command_class_user_code_checksum_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(ADMIN_CODE),
    &zwave_command_class_user_code_admin_code_set,
    &zwave_command_class_user_code_admin_code_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(KEYPAD_MODE),
    &zwave_command_class_user_code_keypad_mode_set,
    &zwave_command_class_user_code_keypad_mode_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(SUPPORTED_FLAGS),
    NULL,
    &zwave_command_class_user_code_capabilities_get);

  // Attribute Resolver event listener:
  register_send_event_handler(ATTRIBUTE(DELETE_ALL_REQUESTED),
                              &on_delete_all_send_data_complete);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler = &zwave_command_class_user_code_control_handler;
  // Not supported, so this does not really matter
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_USER_CODE_V2;
  handler.version                    = USER_CODE_VERSION_V2;
  handler.command_class_name         = "User Code";
  handler.comments                   = "Partial Control";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
