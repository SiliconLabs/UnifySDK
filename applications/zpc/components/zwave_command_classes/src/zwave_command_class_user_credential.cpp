
/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

// System
#include <cstdlib>

#include "zwave_command_class_user_credential.h"
#include "zwave_command_classes_utils.h"
#include "ZW_classcmd.h"

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_command_handler.h"
#include "zwave_command_class_version_types.h"
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store.h"
#include "zwave_controller_crc16.h"

// Unify
#include "attribute_resolver.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_type_registration.h"

// DotDot
#include "unify_dotdot_attribute_store_node_state.h"
#include "dotdot_mqtt_helpers.hpp"

// Cpp related
#include <vector>
#include <string>
#include <map>
#include <boost/format.hpp>
#include <set>

// Cpp Attribute store
#include "attribute.hpp"
#include "zwave_frame_generator.hpp"
#include "zwave_frame_parser.hpp"

// Private helpers
#include "private/user_credential/user_credential_definitions.hpp"
#include "private/user_credential/user_credential_user_capabilities.h"
#include "private/user_credential/user_credential_credential_capabilities.h"
#include "private/user_credential/user_credential_credential_type_capabilities.h"
#include "private/user_credential/user_credential_checksum_calculator.h"
#include "private/user_credential/user_credential_helpers.hpp"

// Using
using namespace user_credential_helpers;

/**
 * @brief Implementation notes
 * 
 * 1. Perform mandatory interview with User and Credentials Capabilities
 * 2. Once interview is finished we retrieve all Users/Credentials (see zwave_network_status_changed)
 *    > To get all Users and Credential we create a new node with a desired value. 
 *    > The lack of reported value will trigger a GET
 *    > GET will set the reported value so we can get a report for this USER/CREDENTIAL
 * 
 */

namespace
{
zwave_frame_generator frame_generator(
  COMMAND_CLASS_USER_CREDENTIAL);  //NOSONAR - false positive since it is warped in a namespace

// Callbacks
std::set<user_credential_slot_changed_callback_t>
  user_credential_slot_changed_callback;  //NOSONAR - false positive since it is warped in a namespace
std::set<user_credential_slot_message_callback_t>
  user_credential_slot_message_callback;  //NOSONAR - false positive since it is warped in a namespace
}  // namespace

/**
 * @brief Keep track of the interview state
 * 
 * User interview start with a get on User 0 to get the first user. 
 * We keep this node to store all the credentials that are not assigned to a user.
 * 
 * So we need to set its reported in the first User Report otherwise the first GET will
 * never be marked as resolved.
 * 
 * We also need to keep track of the next user to interview, so we can first get all the 
 * credential for the current user before moving to the next one.
 */
namespace users_interview_state
{
// WARNING : Use functions instead of those variables
// Next user ID to be interviewed
user_credential_user_unique_id_t next_user_id = 0;
// Special node for user 0
// Used to resolve it in the report instead of in the get to prevent resolver issues
attribute_store::attribute user_0_node = ATTRIBUTE_STORE_INVALID_NODE;

/**
 * @brief Mark an node as the user 0 node
 * 
 * @param node Node to mark as user 0
 */
void set_user_0_node(attribute_store::attribute node)
{  
  sl_log_debug(LOG_TAG, "Starting interview for all users on the device.");
  user_0_node = node;
}

/**
 * @brief Resolve user 0 if needed
 */
void resolve_user_0_node()
{
  if (user_0_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return;
  }

  try {
    user_0_node.set_reported<user_credential_user_unique_id_t>(0);
    user_0_node = ATTRIBUTE_STORE_INVALID_NODE;
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG, "Error while setting user 0 node : %s", e.what());
  }
}

/**
 * @brief Schedule the next user interview
 * 
 * @param user_id User ID to interview next
 */
void schedule_next_user_interview(user_credential_user_unique_id_t user_id)
{
  sl_log_debug(LOG_TAG, "Schedule a get for next User ID :%d", next_user_id);
  next_user_id = user_id;
}

/**
 * @brief Trigger a get for the next user
 * 
 * If there is not next user, we will trigger the all users checksum get
 * (if supported)
 * 
 * @param endpoint_node Current endpoint node
 */
void trigger_get_for_next_user(attribute_store::attribute endpoint_node)
{
  // If we get here it means that we have finished the interview for users and
  // their credentials
  if (next_user_id == 0) {
    sl_log_debug(LOG_TAG, "User interview finished");
    // Check if we supports all users checksum to compute it
    user_credential::user_capabilities capabilities(endpoint_node);
    if (capabilities.is_all_users_checksum_supported()) {
      // This will do nothing if the node already exists
      // Otherwise it will trigger all_users_checksum_get
      endpoint_node.emplace_node(ATTRIBUTE(ALL_USERS_CHECKSUM));
    }
    return;
  }

  sl_log_debug(LOG_TAG, "Trigger a get for next User ID :%d", next_user_id);

  endpoint_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID),
                             next_user_id,
                             DESIRED_ATTRIBUTE);

  // Reset state
  next_user_id = 0;
}
}  // namespace users_interview_state

/////////////////////////////////////////////////////////////////////////////
// Callbacks
/////////////////////////////////////////////////////////////////////////////
void zwave_command_class_user_credential_set_uuic_slot_changed_callback(
  user_credential_slot_changed_callback_t callback)
{
  if (callback != nullptr) {
    user_credential_slot_changed_callback.insert(callback);
  }
}

void zwave_command_class_user_credential_set_message_callback(
  user_credential_slot_message_callback_t callback)
{
  if (callback != nullptr) {
    user_credential_slot_message_callback.insert(callback);
  }
}

void send_message_to_mqtt(sl_log_level level, const std::string &message)
{
  if (level == SL_LOG_DEBUG) {
    sl_log_critical(LOG_TAG, "Debug message should not be sent to MQTT");
    return;
  }
  sl_log(LOG_TAG, level, "%s", message.c_str());

  for (auto &callback: user_credential_slot_message_callback) {
    callback(level, message);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Helpers
/////////////////////////////////////////////////////////////////////////////
std::string
  get_credential_type_debug_str(user_credential_type_t credential_type)
{
  auto fmt = boost::format("Credential Type %1% (%2%)")
             % cred_type_get_enum_value_name(credential_type)
             % static_cast<unsigned int>(credential_type);

  return fmt.str();
}

/////////////////////////////////////////////////////////////////////////////
// Version & Attribute Creation
/////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_user_credential_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  zwave_cc_version_t version = 0;
  attribute_store_get_reported(updated_node, &version, sizeof(version));

  if (version == 0) {
    return;
  }

  sl_log_debug(LOG_TAG, "User Credential version %d", version);

  attribute_store::attribute endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  // The order of the attribute matter since it defines the order of the
  // Z-Wave get command order.
  endpoint_node.emplace_node(ATTRIBUTE(NUMBER_OF_USERS));
  endpoint_node.emplace_node(ATTRIBUTE(SUPPORT_CREDENTIAL_CHECKSUM));
}

/////////////////////////////////////////////////////////////////////////////
// User Credential User Capabilities Get/Report
/////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_user_credential_user_capabilities_get(
  [[maybe_unused]] attribute_store_node_t node,
  uint8_t *frame,
  uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "User Capabilities Get");

  return frame_generator.generate_no_args_frame(USER_CAPABILITIES_GET,
                                                frame,
                                                frame_length);
}

sl_status_t zwave_command_class_user_credential_user_capabilities_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  attribute_store::attribute endpoint_node(
    zwave_command_class_get_endpoint_node(connection_info));

  sl_log_debug(LOG_TAG, "User Capabilities Report");
  const uint8_t expected_size = 9;

  try {
    zwave_frame_parser parser(frame_data, frame_length);

    if (!parser.is_frame_size_valid(expected_size, expected_size + 4)) {
      sl_log_error(LOG_TAG,
                   "Invalid frame size for User Capabilities Report frame");
      return SL_STATUS_FAIL;
    }

    parser.read_sequential<uint16_t>(
      2,
      endpoint_node.emplace_node(ATTRIBUTE(NUMBER_OF_USERS)));
    parser.read_byte(
      endpoint_node.emplace_node(ATTRIBUTE(SUPPORTED_CREDENTIAL_RULES)));
    parser.read_byte(
      endpoint_node.emplace_node(ATTRIBUTE(MAX_USERNAME_LENGTH)));

    parser.read_byte_with_bitmask(
      {{USER_CAPABILITIES_REPORT_PROPERTIES1_USER_SCHEDULE_SUPPORT_BIT_MASK,
        endpoint_node.emplace_node(ATTRIBUTE(SUPPORT_USER_SCHEDULE))},
       {USER_CAPABILITIES_REPORT_PROPERTIES1_ALL_USERS_CHECKSUM_SUPPORT_BIT_MASK,
        endpoint_node.emplace_node(ATTRIBUTE(SUPPORT_ALL_USERS_CHECKSUM))},
       {USER_CAPABILITIES_REPORT_PROPERTIES1_USER_CHECKSUM_SUPPORT_BIT_MASK,
        endpoint_node.emplace_node(ATTRIBUTE(SUPPORT_USER_CHECKSUM))}});

    parser.read_bitmask(
      endpoint_node.emplace_node(ATTRIBUTE(SUPPORTED_USER_TYPES)));

  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while parsing User Capabilities Report frame : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// User Credential Credential Capabilities Get/Report
/////////////////////////////////////////////////////////////////////////////
static sl_status_t
  zwave_command_class_user_credential_credential_capabilities_get(
    [[maybe_unused]] attribute_store_node_t node,
    uint8_t *frame,
    uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "Credential Capabilities Get");

  return frame_generator.generate_no_args_frame(CREDENTIAL_CAPABILITIES_GET,
                                                frame,
                                                frame_length);
}

sl_status_t
  zwave_command_class_user_credential_credential_capabilities_handle_report(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  sl_log_debug(LOG_TAG, "Credential Capabilities Report");

  attribute_store::attribute endpoint_node(
    zwave_command_class_get_endpoint_node(connection_info));

  const uint8_t min_expected_size = 5;

  try {
    zwave_frame_parser parser(frame_data, frame_length);

    // We only needs to check the minimum size here
    if (!parser.is_frame_size_valid(min_expected_size, UINT8_MAX)) {
      sl_log_error(
        LOG_TAG,
        "Invalid frame size for Credential Capabilities Report frame");
      return SL_STATUS_FAIL;
    }

    const uint8_t support_admin_code_bitmask = 0x40;
    auto result                              = parser.read_byte_with_bitmask(
      {{CREDENTIAL_CAPABILITIES_REPORT_PROPERTIES1_CREDENTIAL_CHECKSUM_SUPPORT_BIT_MASK,
                                     endpoint_node.emplace_node(ATTRIBUTE(SUPPORT_CREDENTIAL_CHECKSUM))},
                                    {support_admin_code_bitmask,
                                     endpoint_node.emplace_node(ATTRIBUTE(SUPPORT_ADMIN_PIN_CODE))},
                                    {0x20,
                                     endpoint_node.emplace_node(
          ATTRIBUTE(SUPPORT_ADMIN_PIN_CODE_DEACTIVATION))}});

    if (result[support_admin_code_bitmask]) {
      endpoint_node.emplace_node(ATTRIBUTE(ADMIN_PIN_CODE));
    }

    uint8_t supported_credential_types_count = parser.read_byte();

    // Remove all previous known SUPPORTED_CREDENTIAL_TYPE
    attribute_store::attribute type_node;
    do {
      // Take first supported credential type node
      endpoint_node.child_by_type(ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE))
        .delete_node();
    } while (endpoint_node.child_by_type(ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE))
               .is_valid());

    // Compute this value here since we need it for the exposure of the supported user credential types
    uint16_t ucl_credential_type_mask = 0;

    // Create each node with credential type
    std::vector<attribute_store::attribute> credential_type_nodes;
    for (uint8_t current_credential_type_index = 0;
         current_credential_type_index < supported_credential_types_count;
         current_credential_type_index++) {
      // Create new node
      auto current_credential_type_node
        = endpoint_node.add_node(ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE));
      // Read credential type and save into the node
      auto credential_type = parser.read_byte(current_credential_type_node);
      // Compute bitmask for MQTT
      ucl_credential_type_mask |= (1 << (credential_type - 1));
      // Save the credential type node for later
      credential_type_nodes.push_back(current_credential_type_node);
    }

    // CL Support
    for (uint8_t current_credential_type_index = 0;
         current_credential_type_index < supported_credential_types_count;
         current_credential_type_index++) {
      // Create new node
      auto credential_learn_support_node
        = credential_type_nodes[current_credential_type_index].add_node(
          ATTRIBUTE(CREDENTIAL_LEARN_SUPPORT));
      parser.read_byte_with_bitmask(
        {CREDENTIAL_CAPABILITIES_REPORT_PROPERTIES1_CREDENTIAL_CHECKSUM_SUPPORT_BIT_MASK,
         credential_learn_support_node});
    }

    // Number of Supported Credential Slots
    for (uint8_t current_credential_type_index = 0;
         current_credential_type_index < supported_credential_types_count;
         current_credential_type_index++) {
      auto credential_learn_support_node
        = credential_type_nodes[current_credential_type_index].add_node(
          ATTRIBUTE(CREDENTIAL_SUPPORTED_SLOT_COUNT));

      parser.read_sequential<uint16_t>(2, credential_learn_support_node);
    }

    auto create_and_store_uint8_value = [&](attribute_store_type_t type) {
      for (uint8_t current_credential_type_index = 0;
           current_credential_type_index < supported_credential_types_count;
           current_credential_type_index++) {
        auto node
          = credential_type_nodes[current_credential_type_index].add_node(type);
        parser.read_byte(node);
      }
    };

    create_and_store_uint8_value(ATTRIBUTE(CREDENTIAL_MIN_LENGTH));
    create_and_store_uint8_value(ATTRIBUTE(CREDENTIAL_MAX_LENGTH));
    create_and_store_uint8_value(
      ATTRIBUTE(CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT));
    create_and_store_uint8_value(ATTRIBUTE(CREDENTIAL_LEARN_NUMBER_OF_STEPS));

    // Set UCL mask for supported user credential types
    endpoint_node
      .emplace_node(
        DOTDOT_ATTRIBUTE_ID_USER_CREDENTIAL_SUPPORTED_CREDENTIAL_TYPES)
      .set_reported(ucl_credential_type_mask);
  } catch (const std::exception &e) {
    sl_log_error(
      LOG_TAG,
      "Error while parsing Credential Capabilities Report frame : %s",
      e.what());
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// All User Checksum Get/Report
/////////////////////////////////////////////////////////////////////////////

/**
 * @brief Add given user node to the checksum calculator
 * 
 * @param user_node User node
 * @param checksum_calculator Checksum calculator
 * @param include_user_identifier Include the user identifier in the checksum (used in ALL_USERS_CHECKSUM)
 */
void add_user_node_to_checksum(
  const attribute_store::attribute &user_node,
  user_credential::checksum_calculator &checksum_calculator,
  bool include_user_identifier = false)
{
  // Check if we need to add the identifier to the checksum calculation
  if (include_user_identifier) {
    checksum_calculator.add_node(user_node);
  }

  // Gather all the User values
  const std::vector<attribute_store_type_t> user_attributes = {
    ATTRIBUTE(USER_TYPE),
    ATTRIBUTE(USER_ACTIVE_STATE),
    ATTRIBUTE(CREDENTIAL_RULE),
    ATTRIBUTE(USER_NAME_ENCODING),
    ATTRIBUTE(USER_NAME),
  };

  for (auto attribute: user_attributes) {
    checksum_calculator.add_node(user_node.child_by_type(attribute));
  }

  // The all credential data
  for (auto credential_type_node:
       user_node.children(ATTRIBUTE(CREDENTIAL_TYPE))) {
    for (auto credential_slot_node:
         credential_type_node.children(ATTRIBUTE(CREDENTIAL_SLOT))) {
      if (!credential_slot_node.reported_exists()) {
        sl_log_debug(
          LOG_TAG,
          "%d reported value is not defined. Not adding to checksum.",
          credential_slot_node.value_to_string());
        continue;
      }

      // Add credential type to checksum
      checksum_calculator.add_node(credential_type_node);
      // Add credential slot to checksum
      checksum_calculator.add_node(credential_slot_node);
      checksum_calculator.add_node(
        credential_slot_node.child_by_type(ATTRIBUTE(CREDENTIAL_DATA)));
    }
  }
}

static sl_status_t zwave_command_class_user_credential_all_user_checksum_get(
  [[maybe_unused]] attribute_store_node_t node,
  uint8_t *frame,
  uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "All Users Checksum Get");

  return frame_generator.generate_no_args_frame(ALL_USERS_CHECKSUM_GET,
                                                frame,
                                                frame_length);
}

sl_status_t zwave_command_class_user_credential_all_user_checksum_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  sl_log_debug(LOG_TAG, "All Users Checksum Report");
  constexpr uint8_t expected_size = sizeof(ZW_ALL_USERS_CHECKSUM_REPORT_FRAME);

  attribute_store::attribute endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  try {
    zwave_frame_parser parser(frame_data, frame_length);

    if (!parser.is_frame_size_valid(expected_size)) {
      sl_log_error(LOG_TAG,
                   "Invalid frame size for All User Checksum Report frame");
      return SL_STATUS_NOT_SUPPORTED;
    }

    auto received_checksum = parser.read_sequential<user_credential_checksum_t>(
      2,
      endpoint_node.emplace_node(ATTRIBUTE(ALL_USERS_CHECKSUM)));

    // Create the checksum calculator
    user_credential::checksum_calculator checksum_calculator;

    // Add all user nodes to the checksum
    for (auto user_node: endpoint_node.children(ATTRIBUTE(USER_UNIQUE_ID))) {
      // Ignore user node 0
      if (user_node.reported_exists()
          && user_node.reported<user_credential_user_unique_id_t>() == 0) {
        continue;
      }
      add_user_node_to_checksum(user_node, checksum_calculator, true);
    }

    // Compute the checksum
    auto computed_checksum = checksum_calculator.compute_checksum();
    if (computed_checksum != received_checksum) {
      auto message
        = boost::format(
            "All Users Checksum mismatch. Received %1$#x but computed %2$#x")
          % received_checksum % computed_checksum;
      send_message_to_mqtt(SL_LOG_ERROR, message.str());
      return SL_STATUS_FAIL;
    } else {
      send_message_to_mqtt(SL_LOG_INFO, "All Users Checksum match.");
    }
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while parsing All User Checksum Report frame : %s",
                 e.what());
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Credential Set/Get/Report
/////////////////////////////////////////////////////////////////////////////

/**
 * @brief Trigger a GET credential command
 * 
 * Create credential_type (reported) and credential_slot (desired) nodes if they don't exist
 * 
 * trigger_get_credential(user_node, 0, 0) will trigger a GET command for the first credential of user_node
 * 
 * @param user_unique_id_node User ID node
 * @param credential_type 0 to get the first credential; valid value otherwise
 * @param credential_slot 0 to get the first credential; valid value otherwise
 *
*/
void trigger_get_credential(attribute_store::attribute &user_unique_id_node,
                            user_credential_type_t credential_type,
                            user_credential_slot_t credential_slot)
{
  sl_log_debug(LOG_TAG,
               "Trigger GET credential for user %d : "
               "Credential type %s, credential slot %d",
               user_unique_id_node.reported<user_credential_user_unique_id_t>(),
               get_credential_type_debug_str(credential_type).c_str(),
               credential_slot);
  user_unique_id_node
    .emplace_node(ATTRIBUTE(CREDENTIAL_TYPE),
                  credential_type,
                  REPORTED_ATTRIBUTE)
    .emplace_node(ATTRIBUTE(CREDENTIAL_SLOT),
                  credential_slot,
                  DESIRED_ATTRIBUTE);
}

sl_status_t zwave_command_class_user_credential_credential_set(
  attribute_store_node_t credential_operation_type_node,
  uint8_t *frame,
  uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "Credential Set");
  try {
    auto cred_nodes
      = get_credential_identifier_nodes(credential_operation_type_node);

    auto operation_type
      = cred_nodes.slot_node.child_by_type(ATTRIBUTE(CREDENTIAL_OPERATION_TYPE))
          .desired<user_credential_operation_type_t>();

    sl_log_debug(LOG_TAG, "Operation type : %d", operation_type);

    // Generate the frame
    const bool is_delete_operation
      = (operation_type == CREDENTIAL_SET_OPERATION_TYPE_DELETE);

    uint8_t expected_frame_size = 9;
    uint8_t credential_size     = 0;
    auto credential_data_node
      = cred_nodes.slot_node.child_by_type(ATTRIBUTE(CREDENTIAL_DATA));

    if (!is_delete_operation) {
      auto state = REPORTED_ATTRIBUTE;
      if (credential_data_node.desired_exists()) {
        state = DESIRED_ATTRIBUTE;
      }
      credential_size = static_cast<uint8_t>(
        credential_data_node.get<std::vector<uint8_t>>(state).size());
    }

    // Append the credential data
    expected_frame_size += credential_size;

    frame_generator.initialize_frame(CREDENTIAL_SET,
                                     frame,
                                     expected_frame_size);
    frame_generator.add_value(cred_nodes.user_unique_id_node,
                              DESIRED_OR_REPORTED_ATTRIBUTE);
    frame_generator.add_value(cred_nodes.type_node,
                              DESIRED_OR_REPORTED_ATTRIBUTE);
    frame_generator.add_value(cred_nodes.slot_node,
                              DESIRED_OR_REPORTED_ATTRIBUTE);
    frame_generator.add_raw_byte(operation_type);
    frame_generator.add_raw_byte(credential_size);
    if (!is_delete_operation) {
      frame_generator.add_value(credential_data_node,
                                DESIRED_OR_REPORTED_ATTRIBUTE);
    }

    frame_generator.validate_frame(frame_length);

  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while generating Credential Set frame : %s",
                 e.what());
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_user_credential_credential_get(
  attribute_store_node_t credential_slot_node,
  uint8_t *frame,
  uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "Credential Get");

  // Generate the frame
  constexpr auto expected_frame_size
    = static_cast<uint8_t>(sizeof(ZW_CREDENTIAL_GET_FRAME));
  try {
    auto cred_nodes = get_credential_identifier_nodes(credential_slot_node);

    frame_generator.initialize_frame(CREDENTIAL_GET,
                                     frame,
                                     expected_frame_size);
    frame_generator.add_value(cred_nodes.user_unique_id_node,
                              REPORTED_ATTRIBUTE);
    frame_generator.add_value(cred_nodes.type_node, REPORTED_ATTRIBUTE);
    frame_generator.add_value(cred_nodes.slot_node, DESIRED_ATTRIBUTE);
    frame_generator.validate_frame(frame_length);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while generating Credential Get frame : %s",
                 e.what());
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

enum class credential_report_type_t : uint8_t {
  CREDENTIAL_ADDED                          = 0x00,
  CREDENTIAL_MODIFIED                       = 0x01,
  CREDENTIAL_DELETED                        = 0x02,
  CREDENTIAL_UNCHANGED                      = 0x03,
  RESPONSE_TO_GET                           = 0x04,
  CREDENTIAL_ADD_REJECTED_LOCATION_OCCUPIED = 0x05,
  CREDENTIAL_MODIFY_REJECTED_LOCATION_EMPTY = 0x06,
  CREDENTIAL_DUPLICATE_ERROR                = 0x07,
  CREDENTIAL_MANUFACTURER_SECURITY_RULE     = 0x08,
  CREDENTIAL_LOCATION_ALREADY_ASSIGNED      = 0x09,
  CREDENTIAL_DUPLICATE_ADMIN_CODE           = 0x0A
};

sl_status_t
  handle_credential_deletion(attribute_store::attribute& endpoint_node,
                             attribute_store::attribute& user_id_node,
                             user_credential_user_unique_id_t user_id,
                             user_credential_type_t credential_type,
                             user_credential_user_unique_id_t credential_slot)
{
  if (user_id != 0 && credential_type != 0 && credential_slot != 0) {
    sl_log_info(LOG_TAG,
                "Credential Deleted. Type %s, Slot %d (User %d)",
                get_credential_type_debug_str(credential_type).c_str(),
                credential_slot,
                user_id);
    // Delete the credential slot node
    get_credential_identifier_nodes(endpoint_node,
                                    {user_id, REPORTED_ATTRIBUTE},
                                    {credential_type, REPORTED_ATTRIBUTE},
                                    {credential_slot, REPORTED_ATTRIBUTE})
      .slot_node.delete_node();
  } else if (user_id != 0 && credential_type != 0 && credential_slot == 0) {
    sl_log_info(LOG_TAG,
                "All credential type %s deleted for user %d.",
                get_credential_type_debug_str(credential_type).c_str(),
                user_id);
    for_each_credential_type_nodes_for_user(
      user_id_node,
      [](attribute_store::attribute &credential_type_node) {
        credential_type_node.delete_node();
      },
      credential_type);
  } else if (user_id != 0 && credential_type == 0 && credential_slot == 0) {
    sl_log_info(LOG_TAG, "All credentials for user %d deleted.", user_id);
    for_each_credential_type_nodes_for_user(
      user_id_node,
      [](attribute_store::attribute &credential_type_node) {
        credential_type_node.delete_node();
      });
  } else if (user_id == 0 && credential_type == 0 && credential_slot == 0) {
    sl_log_info(LOG_TAG, "All credentials deleted.");
    for_each_credential_type_nodes(
      endpoint_node,
      [](attribute_store::attribute &credential_type_node) {
        credential_type_node.delete_node();
      });
  } else if (user_id == 0 && credential_type != 0 && credential_slot == 0) {
    sl_log_info(LOG_TAG,
                "All credentials of type %s are deleted",
                get_credential_type_debug_str(credential_type).c_str());
    for_each_credential_type_nodes(
      endpoint_node,
      [](attribute_store::attribute &credential_type_node) {
        credential_type_node.delete_node();
      },
      credential_type);
  } else {
    sl_log_critical(LOG_TAG,
                    "Invalid combination of user_id %d, credential_type %s and "
                    "credential_slot %d for credential deletion",
                    user_id,
                    get_credential_type_debug_str(credential_type).c_str(),
                    credential_slot);
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_credential_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  sl_log_debug(LOG_TAG, "Credential Report");

  attribute_store::attribute endpoint_node(
    zwave_command_class_get_endpoint_node(connection_info));

  const uint8_t min_size = 15;

  try {
    zwave_frame_parser parser(frame_data, frame_length);

    if (!parser.is_frame_size_valid(min_size, UINT8_MAX)) {
      sl_log_error(LOG_TAG, "Invalid frame size for Credential Report frame");
      return SL_STATUS_FAIL;
    }

    credential_report_type_t credential_report_type
      = static_cast<credential_report_type_t>(parser.read_byte());
    auto user_id = parser.read_sequential<user_credential_user_unique_id_t>(2);
    user_credential_type_t credential_type = parser.read_byte();
    auto credential_slot = parser.read_sequential<user_credential_slot_t>(2);

    sl_log_debug(LOG_TAG,
                 "Credential Report (%d). %s, Slot %d (User %d)",
                 credential_report_type,
                 get_credential_type_debug_str(credential_type).c_str(),
                 credential_slot,
                 user_id);

    // Helper function to clean up pending credentials slot nodes
    auto clean_up_pending_credentials_slot_nodes = [&]() {
      try {
        auto nodes = get_credential_identifier_nodes(
          endpoint_node,
          {user_id, REPORTED_ATTRIBUTE},
          {credential_type, DESIRED_OR_REPORTED_ATTRIBUTE},
          {credential_slot, DESIRED_ATTRIBUTE});

        nodes.slot_node.delete_node();
        sl_log_debug(LOG_TAG,
                     "Cleaning temporary credential slot node : %d (credential "
                     "type %s, user %d)",
                     credential_slot,
                     get_credential_type_debug_str(credential_type).c_str(),
                     user_id);
      } catch (...) {
        // Try again with reported attribute
        // That should not trigger an error, but if it does it means that something went wrong
        auto nodes = get_credential_identifier_nodes(
          endpoint_node,
          {user_id, REPORTED_ATTRIBUTE},
          {credential_type, DESIRED_OR_REPORTED_ATTRIBUTE},
          {credential_slot, REPORTED_ATTRIBUTE});

        sl_log_debug(
          LOG_TAG,
          "Cleaning desired values of credential slot node : %d (credential "
          "type %s, user %d)",
          credential_slot,
          get_credential_type_debug_str(credential_type).c_str(),
          user_id);

        for (auto child: nodes.slot_node.children()) {
          child.clear_desired();
        }
      }
    };

    // We should have a valid user id if we receive this report
    auto user_id_node
      = get_user_unique_id_node(endpoint_node, user_id, REPORTED_ATTRIBUTE);

    attribute_store::attribute credential_type_node;
    attribute_store::attribute credential_slot_node;

    switch (credential_report_type) {
      case credential_report_type_t::CREDENTIAL_ADDED:
        // In case of credential learn, the credential is already present in the
        // node tree. So we don't check if the node exists before creating it.
        // Only update existing nodes if they are present.
        credential_type_node
          = create_or_update_desired_value(user_id_node,
                                           ATTRIBUTE(CREDENTIAL_TYPE),
                                           credential_type);
        credential_slot_node
          = create_or_update_desired_value(credential_type_node,
                                           ATTRIBUTE(CREDENTIAL_SLOT),
                                           credential_slot);
        break;
      case credential_report_type_t::CREDENTIAL_MODIFIED: {
        // Should throw an exception if the credential doesn't exists
        auto nodes = get_credential_identifier_nodes(
          endpoint_node,
          {user_id, REPORTED_ATTRIBUTE},
          {credential_type, REPORTED_ATTRIBUTE},
          {credential_slot, REPORTED_ATTRIBUTE});
        credential_type_node = nodes.type_node;
        credential_slot_node = nodes.slot_node;
        // Clear desired value
        credential_slot_node.clear_desired();
        credential_slot_node.set_reported(credential_slot);
      } break;
      case credential_report_type_t::CREDENTIAL_DELETED:
        return handle_credential_deletion(endpoint_node,
                                          user_id_node,
                                          user_id,
                                          credential_type,
                                          credential_slot);
      case credential_report_type_t::CREDENTIAL_UNCHANGED:
        send_message_to_mqtt(SL_LOG_INFO,
                             "Credential Unchanged. Type "
                               + get_credential_type_debug_str(credential_type)
                               + ", Slot " + std::to_string(credential_slot)
                               + " (User " + std::to_string(user_id) + ")");
        return SL_STATUS_OK;
      // Update desired value if found, otherwise create the nodes
      case credential_report_type_t::RESPONSE_TO_GET:
        // First check if this is the first credential interview
        credential_type_node
          = user_id_node.child_by_type_and_value(ATTRIBUTE(CREDENTIAL_TYPE),
                                                 0,
                                                 REPORTED_ATTRIBUTE);
        credential_slot_node = credential_type_node.child_by_type_and_value(
          ATTRIBUTE(CREDENTIAL_SLOT),
          0,
          DESIRED_ATTRIBUTE);
        // Update the values of the first nodes if present
        if (credential_type_node.is_valid()
            && credential_slot_node.is_valid()) {
          // If no credential are reported, we can delete the nodes
          if (credential_type == 0 && credential_slot == 0) {
            sl_log_debug(LOG_TAG, "No credential to get for user %d", user_id);
            credential_type_node.delete_node();
            credential_slot_node.delete_node();
            return SL_STATUS_OK;
          } else {
            sl_log_debug(LOG_TAG,
                         "First credential : Updating credential type and slot "
                         "for user %d",
                         user_id);
            credential_type_node.set_reported(credential_type);
            credential_slot_node.clear_desired();
            credential_slot_node.set_reported(credential_slot);
          }
        } else {
          // Otherwise create or get the nodes
          credential_type_node
            = create_or_update_desired_value(user_id_node,
                                             ATTRIBUTE(CREDENTIAL_TYPE),
                                             credential_type);
          credential_slot_node
            = create_or_update_desired_value(credential_type_node,
                                             ATTRIBUTE(CREDENTIAL_SLOT),
                                             credential_slot);
        }
        break;
      case credential_report_type_t::CREDENTIAL_ADD_REJECTED_LOCATION_OCCUPIED:
        sl_log_error(LOG_TAG,
                     "Credential data rejected as it already exists : user %d, "
                     "credential type %s, credential slot %d",
                     user_id,
                     get_credential_type_debug_str(credential_type).c_str(),
                     credential_slot);
        clean_up_pending_credentials_slot_nodes();
        return SL_STATUS_OK;
      case credential_report_type_t::CREDENTIAL_MODIFY_REJECTED_LOCATION_EMPTY:
        sl_log_error(
          LOG_TAG,
          "Credential data cannot be modified as it does not exists : user %d, "
          "credential type %s, credential slot %d",
          user_id,
          get_credential_type_debug_str(credential_type).c_str(),
          credential_slot);

        credential_type_node
          = user_id_node.child_by_type_and_value(ATTRIBUTE(CREDENTIAL_TYPE),
                                                 credential_type,
                                                 DESIRED_OR_REPORTED_ATTRIBUTE);

        if (!credential_type_node.is_valid()) {
          sl_log_debug(
            LOG_TAG,
            "No credential type found for user %d, credential type %s",
            user_id,
            get_credential_type_debug_str(credential_type).c_str());
          return SL_STATUS_OK;
        }

        credential_slot_node = credential_type_node.child_by_type_and_value(
          ATTRIBUTE(CREDENTIAL_SLOT),
          credential_slot,
          DESIRED_ATTRIBUTE);

        credential_slot_node.delete_node();
        return SL_STATUS_OK;
      // Duplicate Credential : 0x02
      case credential_report_type_t::CREDENTIAL_DUPLICATE_ERROR: {
        // Do nothing, the credential GET will clean up for us
        sl_log_warning(LOG_TAG,
                       "Duplicate Credential (Already present for user %d, "
                       "credential type %s, "
                       "credential slot %d)",
                       user_id,
                       get_credential_type_debug_str(credential_type).c_str(),
                       credential_slot);

        // So this is the fun part when we hunt down the faulty credential slot node
        parser.read_byte();  // We don't care about this one
        uint8_t cred_data_size = parser.read_byte();
        auto duplicate_data
          = parser.read_sequential<std::vector<uint8_t>>(cred_data_size);

        bool found_duplicate = false;
        // We need to find the credential slot node that contains the same data
        for_each_credential_type_nodes(
          endpoint_node,
          [&found_duplicate, &duplicate_data](
            attribute_store::attribute &current_credential_type_node) {
            for (auto &current_credential_slot_node:
                 current_credential_type_node.children(
                   ATTRIBUTE(CREDENTIAL_SLOT))) {
              if (current_credential_slot_node.desired_exists()) {
                auto current_data
                  = current_credential_slot_node
                      .child_by_type(ATTRIBUTE(CREDENTIAL_DATA))
                      .get<std::vector<uint8_t>>(DESIRED_OR_REPORTED_ATTRIBUTE);
                if (
                  current_data
                  == duplicate_data) {  //NOSONAR - Don't have a better way to write this
                  sl_log_debug(LOG_TAG,
                               "Found the faulty credential slot node : %d",
                               current_credential_slot_node
                                 .desired<user_credential_slot_t>());
                  current_credential_slot_node.delete_node();
                  found_duplicate = true;
                  return;
                }
              }
            }
          },
          credential_type);

        if (!found_duplicate) {
          // If we are here that means we din't find the duplicate data
          sl_log_error(LOG_TAG,
                       "Couldn't find the duplicate credential slot node.");
          return SL_STATUS_FAIL;
        } else {
          return SL_STATUS_OK;
        }
      }
      case credential_report_type_t::CREDENTIAL_MANUFACTURER_SECURITY_RULE:
        sl_log_warning(
          LOG_TAG,
          "Credential data rejected as it doesn't respect manufacturer "
          "security rules : user %d, credential type %s, "
          "credential slot %d",
          user_id,
          get_credential_type_debug_str(credential_type).c_str(),
          credential_slot);
        // This should contains the faulty credential
        clean_up_pending_credentials_slot_nodes();
        return SL_STATUS_OK;
      case credential_report_type_t::CREDENTIAL_LOCATION_ALREADY_ASSIGNED:
        sl_log_warning(
          LOG_TAG,
          "Credential data rejected as location is already assigned : user %d, "
          "credential type %s, credential slot %d",
          user_id,
          get_credential_type_debug_str(credential_type).c_str(),
          credential_slot);
        // This should contains the faulty credential
        clean_up_pending_credentials_slot_nodes();
        return SL_STATUS_OK;
      case credential_report_type_t::CREDENTIAL_DUPLICATE_ADMIN_CODE:
        sl_log_warning(LOG_TAG,
                       "Credential data rejected as the new credential is the "
                       "same as admin code");
        // This should contains the faulty credential
        clean_up_pending_credentials_slot_nodes();
        break;
      default:
        sl_log_error(LOG_TAG,
                     "Invalid credential report type %d",
                     credential_report_type);
        return SL_STATUS_FAIL;
    }

    if (!credential_type_node.is_valid()) {
      sl_log_critical(LOG_TAG,
                      "Credential type is invalid when it should be. Can't "
                      "process credential report.");
      return SL_STATUS_FAIL;
    }

    if (!credential_slot_node.is_valid()) {
      sl_log_critical(LOG_TAG,
                      "Credential slot is invalid when it should be. Can't "
                      "process credential report.");
      return SL_STATUS_FAIL;
    }

    // If we are here it means that we have a valid credential type and slot node
    parser.read_byte_with_bitmask(
      {CREDENTIAL_REPORT_PROPERTIES1_CRB_BIT_MASK,
       credential_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_READ_BACK))});

    uint8_t cred_data_size = parser.read_byte();
    parser.read_sequential<std::vector<uint8_t>>(
      cred_data_size,
      credential_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_DATA)));
    parser.read_byte(
      credential_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_MODIFIER_TYPE)));
    parser.read_sequential<user_credential_modifier_node_id_t>(
      2,
      credential_slot_node.emplace_node(
        ATTRIBUTE(CREDENTIAL_MODIFIER_NODE_ID)));

    // Next
    user_credential_type_t next_credential_type = parser.read_byte();
    auto next_credential_slot
      = parser.read_sequential<user_credential_slot_t>(2);

    // Interview next credential is available
    if (next_credential_type != 0 && next_credential_slot != 0
        && credential_report_type
             == credential_report_type_t::RESPONSE_TO_GET) {
      trigger_get_credential(user_id_node,
                             next_credential_type,
                             next_credential_slot);
    } else {
      sl_log_debug(LOG_TAG, "No more credential to get.");
      // Trigger the next user interview if any
      users_interview_state::trigger_get_for_next_user(endpoint_node);
    }

  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while parsing Credential Report frame : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Credential Learn Start/Report/Stop
/////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_user_credential_credential_learn_start(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "Credential Learn Start");
  attribute_store::attribute credential_operation_type_node(node);

  try {
    frame_generator.initialize_frame(CREDENTIAL_LEARN_START,
                                     frame,
                                     sizeof(ZW_CREDENTIAL_LEARN_START_FRAME));
    auto nodes
      = get_credential_identifier_nodes(credential_operation_type_node);

    frame_generator.add_value(nodes.user_unique_id_node,
                              DESIRED_OR_REPORTED_ATTRIBUTE);
    frame_generator.add_value(nodes.type_node, DESIRED_OR_REPORTED_ATTRIBUTE);
    frame_generator.add_value(nodes.slot_node, DESIRED_OR_REPORTED_ATTRIBUTE);
    frame_generator.add_value(credential_operation_type_node,
                              DESIRED_ATTRIBUTE);
    frame_generator.add_value(
      nodes.slot_node.child_by_type(ATTRIBUTE(CREDENTIAL_LEARN_TIMEOUT)),
      DESIRED_OR_REPORTED_ATTRIBUTE);
    frame_generator.validate_frame(frame_length);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while generating Credential Learn Start frame : %s",
                 e.what());
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_user_credential_credential_learn_cancel(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "Credential Learn Cancel");
  attribute_store::attribute credential_learn_stop_node(node);

  credential_learn_stop_node.set_reported<uint8_t>(1);
  credential_learn_stop_node.clear_desired();

  return frame_generator.generate_no_args_frame(CREDENTIAL_LEARN_CANCEL,
                                                frame,
                                                frame_length);
}

sl_status_t zwave_command_class_user_credential_credential_learn_status_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  attribute_store::attribute endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  try {
    zwave_frame_parser parser(frame_data, frame_length);

    if (!parser.is_frame_size_valid(sizeof(ZW_CREDENTIAL_LEARN_REPORT_FRAME))) {
      sl_log_error(
        LOG_TAG,
        "Invalid frame size for Credential Learn Status Report frame");
      return SL_STATUS_NOT_SUPPORTED;
    }

    // Parse the frame
    const uint8_t learn_status = parser.read_byte();
    const auto user_id
      = parser.read_sequential<user_credential_user_unique_id_t>(2);
    const user_credential_type_t credential_type = parser.read_byte();
    const auto credential_slot
      = parser.read_sequential<user_credential_slot_t>(2);
    const uint8_t step_remaining = parser.read_byte();

    sl_log_debug(LOG_TAG,
                 "Credential Learn Status Report. Credential Type: %s / "
                 "Credential Slot: %d (User %d)",
                 cred_type_get_enum_value_name(credential_type).c_str(),
                 credential_slot,
                 user_id);

    auto credential_id_nodes = get_credential_identifier_nodes(
      endpoint_node,
      {user_id, REPORTED_ATTRIBUTE},
      {credential_type, DESIRED_OR_REPORTED_ATTRIBUTE},
      {credential_slot, DESIRED_OR_REPORTED_ATTRIBUTE});

    // Get operation type so we can handle error cases
    // We use desired or reported here since it can be set to reported by the
    // supervision process.
    auto operation_type
      = credential_id_nodes.slot_node
          .child_by_type(ATTRIBUTE(CREDENTIAL_LEARN_OPERATION_TYPE))
          .desired_or_reported<user_credential_operation_type_t>();

    // Action based of current learn status
    std::string learn_status_str;
    bool need_deletion       = false;
    sl_log_level_t log_level = SL_LOG_INFO;

    switch (learn_status) {
      case CREDENTIAL_LEARN_REPORT_STARTED:
        learn_status_str = "Credential Learn Started";
        break;
      case CREDENTIAL_LEARN_REPORT_SUCCESS:
        learn_status_str = "Credential Learn Success";
        break;
      case CREDENTIAL_LEARN_REPORT_ALREADY_IN_PROGRESS:
        log_level        = SL_LOG_WARNING;
        learn_status_str = "Credential Learn already in progress";
        break;
      case CREDENTIAL_LEARN_REPORT_ENDED_NOT_DUE_TO_TIMEOUT:
        log_level        = SL_LOG_ERROR;
        learn_status_str = "Credential Learn Ended (not due to timeout)";
        need_deletion = (operation_type == USER_CREDENTIAL_OPERATION_TYPE_ADD);
        break;
      case CREDENTIAL_LEARN_REPORT_TIMEOUT:
        log_level        = SL_LOG_ERROR;
        learn_status_str = "Credential Learn Timeout";
        need_deletion = (operation_type == USER_CREDENTIAL_OPERATION_TYPE_ADD);
        break;
      case 0x05:  // Credential Learn Step Retry
        learn_status_str = "Credential Learn Step Needs a Retry";
        need_deletion = (operation_type == USER_CREDENTIAL_OPERATION_TYPE_ADD);
        break;
      case CREDENTIAL_LEARN_REPORT_INVALID_CREDENTIAL_LEARN_ADD_OPERATION_TYPE:
        log_level        = SL_LOG_ERROR;
        learn_status_str = "Invalid Add Operation Types";
        break;
      case CREDENTIAL_LEARN_REPORT_INVALID_CREDENTIAL_LEARN_MODIFY_OPERATION_TYPE:
        log_level        = SL_LOG_ERROR;
        learn_status_str = "Invalid Modify Operation Type";
        need_deletion = (operation_type == USER_CREDENTIAL_OPERATION_TYPE_ADD);
        break;
      default:
        learn_status_str
          = "Unknown Credential Learn Status " + std::to_string(learn_status);
        log_level = SL_LOG_CRITICAL;
    }

    auto message = boost::format("Credential Learn Status Report : %1% for "
                                 "Credential Type %2% (Slot %3%, User %4%)")
                   % learn_status_str
                   % cred_type_get_enum_value_name(credential_type)
                   % credential_slot % user_id;
    send_message_to_mqtt(log_level, message.str());

    if (need_deletion) {
      credential_id_nodes.slot_node.delete_node();
      return SL_STATUS_OK;
    }

    // Update nodes
    credential_id_nodes.slot_node.emplace_node(
      ATTRIBUTE(CREDENTIAL_LEARN_STEPS_REMAINING),
      step_remaining);
    credential_id_nodes.slot_node.emplace_node(
      ATTRIBUTE(CREDENTIAL_LEARN_STATUS),
      learn_status);

  } catch (const std::exception &e) {
    sl_log_error(
      LOG_TAG,
      "Error while parsing Credential Learn Status Report frame : %s",
      e.what());

    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// User Unique Identifier Credential Association Set/Report
/////////////////////////////////////////////////////////////////////////////

static sl_status_t zwave_command_class_user_credential_uuic_association_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG,
               "User Unique Identifier Credential Association Set command");
  attribute_store::attribute destination_credential_slot_node(node);

  try {
    frame_generator.initialize_frame(
      USER_CREDENTIAL_ASSOCIATION_SET,
      frame,
      sizeof(ZW_USER_CREDENTIAL_ASSOCIATION_SET_FRAME));

    auto nodes
      = get_credential_identifier_nodes(destination_credential_slot_node);
    frame_generator.add_value(nodes.user_unique_id_node, REPORTED_ATTRIBUTE);
    frame_generator.add_value(nodes.type_node, REPORTED_ATTRIBUTE);
    frame_generator.add_value(nodes.slot_node, REPORTED_ATTRIBUTE);
    frame_generator.add_value(
      nodes.slot_node.child_by_type(ATTRIBUTE(ASSOCIATION_DESTINATION_USER_ID)),
      DESIRED_ATTRIBUTE);
    frame_generator.add_value(destination_credential_slot_node,
                              DESIRED_ATTRIBUTE);
    frame_generator.validate_frame(frame_length);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while generating User Unique Identifier Credential "
                 "Association Set frame : %s",
                 e.what());
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_uuic_association_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  sl_log_debug(LOG_TAG, "UUIC Association Report");

  attribute_store::attribute endpoint_node(
    zwave_command_class_get_endpoint_node(connection_info));

  const uint8_t expected_size
    = sizeof(ZW_USER_CREDENTIAL_ASSOCIATION_REPORT_FRAME);

  try {
    zwave_frame_parser parser(frame_data, frame_length);

    if (!parser.is_frame_size_valid(expected_size)) {
      sl_log_error(LOG_TAG,
                   "Invalid frame size for UUIC Association Report frame");
      return SL_STATUS_FAIL;
    }

    // Parse the frame
    const auto source_user_id
      = parser.read_sequential<user_credential_user_unique_id_t>(2);
    const user_credential_type_t source_credential_type = parser.read_byte();
    const auto source_credential_slot
      = parser.read_sequential<user_credential_slot_t>(2);
    const auto destination_user_id
      = parser.read_sequential<user_credential_user_unique_id_t>(2);
    const auto destination_credential_slot
      = parser.read_sequential<user_credential_slot_t>(2);
    const uint8_t association_status = parser.read_byte();

    sl_log_debug(LOG_TAG,
                 "User Unique Identifier Credential Association Report.");
    sl_log_debug(LOG_TAG, "\tSource User ID: %d", source_user_id);
    sl_log_debug(LOG_TAG,
                 "\tSource Credential Type: %d",
                 source_credential_type);
    sl_log_debug(LOG_TAG,
                 "\tSource Credential Slot: %d",
                 source_credential_slot);
    sl_log_debug(LOG_TAG, "\tDestination User ID: %d", destination_user_id);
    sl_log_debug(LOG_TAG,
                 "\tDestination Credential Slot: %d",
                 destination_credential_slot);

    // Get nodes
    auto source_credential_nodes = get_credential_identifier_nodes(
      endpoint_node,
      {source_user_id, REPORTED_ATTRIBUTE},
      {source_credential_type, REPORTED_ATTRIBUTE},
      {source_credential_slot, REPORTED_ATTRIBUTE});

    // Set association status
    auto source_credential_slot_node = source_credential_nodes.slot_node;
    source_credential_slot_node.emplace_node(ATTRIBUTE(ASSOCIATION_STATUS))
      .set_reported(association_status);

    // Clean up association data so ZPC won't try to send the SET command again
    std::vector<attribute_store_type_t> attributes_to_be_deleted
      = {ATTRIBUTE(ASSOCIATION_DESTINATION_USER_ID),
         ATTRIBUTE(ASSOCIATION_DESTINATION_CREDENTIAL_SLOT)};
    for (auto attribute_type: attributes_to_be_deleted) {
      source_credential_slot_node.child_by_type(attribute_type).delete_node();
    }

    // If something went wrong end device side, log the error and return
    // This should handle the slot already taken case
    if (association_status != USER_CREDENTIAL_ASSOCIATION_REPORT_SUCCESS) {
      send_message_to_mqtt(
        SL_LOG_ERROR,
        "User Unique Identifier Credential Association error. "
        "Reported status code : "
          + std::to_string(association_status));
      return SL_STATUS_OK;
    }

    send_message_to_mqtt(
      SL_LOG_INFO,
      (boost::format("Credential Slot %1% (%2%) (user %3%) moved to "
                     "Credential slot %4% (user %5%)")
       % source_credential_slot
       % cred_rule_get_enum_value_name(source_credential_type) % source_user_id
       % destination_credential_slot % destination_user_id)
        .str());

    source_credential_slot_node.set_reported(destination_credential_slot);

    // Need to move to new user
    if (destination_user_id != source_user_id) {
      sl_log_debug(LOG_TAG, "Moving slot to new user");

      // Get destination user node
      auto destination_user_id_node
        = get_user_unique_id_node(endpoint_node,
                                  destination_user_id,
                                  REPORTED_ATTRIBUTE);

      // Get new parent
      auto destination_credential_type_node
        = destination_user_id_node.emplace_node(ATTRIBUTE(CREDENTIAL_TYPE),
                                                source_credential_type);

      sl_status_t result = source_credential_slot_node.change_parent(
        destination_credential_type_node);
      if (result != SL_STATUS_OK) {
        send_message_to_mqtt(
          SL_LOG_ERROR,
          (boost::format(
             "Error while moving slot %1% (user %2%) to slot %3% (user %4%)")
           % source_credential_slot % source_user_id
           % destination_credential_slot % destination_user_id)
            .str());
        return result;
      }
    }

    // Call MQTT callback if needed to notify that the slot has changed
    for (auto &uicc_slot_changed_callback:
         user_credential_slot_changed_callback) {
      uicc_slot_changed_callback(
        {source_user_id, source_credential_type, source_credential_slot},
        source_credential_slot_node);
    }
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while parsing UUIC Association Report frame : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// User Set/Get/Report
/////////////////////////////////////////////////////////////////////////////

static sl_status_t zwave_command_class_user_credential_user_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  try {
    // Node setup
    attribute_store::attribute user_operation_type_node(node);
    attribute_store::attribute user_unique_id_node
      = user_operation_type_node.first_parent(ATTRIBUTE(USER_UNIQUE_ID));

    // Get Values
    auto user_unique_id
      = user_unique_id_node
          .desired_or_reported<user_credential_user_unique_id_t>();
    auto user_operation_type
      = user_operation_type_node.desired<user_credential_operation_type_t>();

    const bool is_delete_operation
      = (user_operation_type == USER_SET_OPERATION_TYPE_DELETE);

    uint8_t expected_frame_size = is_delete_operation ? 5 : 12;
    uint8_t user_name_size      = 0;

    if (!is_delete_operation) {
      user_name_size = static_cast<uint8_t>(
        user_unique_id_node.child_by_type(ATTRIBUTE(USER_NAME))
          .get<std::string>(DESIRED_OR_REPORTED_ATTRIBUTE)
          .size());
    }

    // Append the user name size (will be 0 if is delete operation)
    expected_frame_size += user_name_size;

    sl_log_debug(LOG_TAG,
                 "User SET for user %d (operation type : %d)",
                 user_unique_id,
                 user_operation_type);

    // Creating the frame
    frame_generator.initialize_frame(USER_SET, frame, expected_frame_size);

    frame_generator.add_value(user_operation_type_node, DESIRED_ATTRIBUTE);
    frame_generator.add_value(user_unique_id_node,
                              DESIRED_OR_REPORTED_ATTRIBUTE);

    if (!is_delete_operation) {
      frame_generator.add_value(
        user_unique_id_node.child_by_type(ATTRIBUTE(USER_TYPE)),
        DESIRED_OR_REPORTED_ATTRIBUTE);
      frame_generator.add_value(
        user_unique_id_node.child_by_type(ATTRIBUTE(USER_ACTIVE_STATE)),
        DESIRED_OR_REPORTED_ATTRIBUTE);
      frame_generator.add_value(
        user_unique_id_node.child_by_type(ATTRIBUTE(CREDENTIAL_RULE)),
        DESIRED_OR_REPORTED_ATTRIBUTE);
      frame_generator.add_value(user_unique_id_node.child_by_type(
                                  ATTRIBUTE(USER_EXPIRING_TIMEOUT_MINUTES)),
                                DESIRED_OR_REPORTED_ATTRIBUTE);
      frame_generator.add_value(
        user_unique_id_node.child_by_type(ATTRIBUTE(USER_NAME_ENCODING)),
        DESIRED_OR_REPORTED_ATTRIBUTE);
      frame_generator.add_raw_byte(user_name_size);
      frame_generator.add_value(
        user_unique_id_node.child_by_type(ATTRIBUTE(USER_NAME)),
        DESIRED_OR_REPORTED_ATTRIBUTE);
    }

    frame_generator.validate_frame(frame_length);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while generating User SET frame : %s",
                 e.what());
    return SL_STATUS_NOT_SUPPORTED;
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_user_credential_user_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  attribute_store::attribute user_unique_id_node(node);

  // If we enter this state it means that something went badly wrong or
  // user initiate the interview process again.
  // In both cases we want to invalidate the user database so that the device
  // can send us the correct user database.

  if (!user_unique_id_node.desired_exists()) {
    sl_log_warning(LOG_TAG,
                   "Can't get user unique id Desired value. Removing all users "
                   "to perform interview again.");
    attribute_store::attribute endpoint_node = user_unique_id_node.parent();

    // Get User node count
    for (auto user_node: endpoint_node.children(ATTRIBUTE(USER_UNIQUE_ID))) {
      attribute_store_delete_node(user_node);
    }

    // NOTE : In the case of user re-interviewing the device, it will be interviewed again when the node goes ONLINE.
    return SL_STATUS_NOT_SUPPORTED;
  }

  user_credential_user_unique_id_t user_id
    = user_unique_id_node.desired<user_credential_user_unique_id_t>();
  sl_log_debug(LOG_TAG, "User Get for user %d", user_id);

  // Generate the frame
  constexpr uint8_t expected_frame_size
    = static_cast<uint8_t>(sizeof(ZW_USER_GET_FRAME));
  try {
    frame_generator.initialize_frame(USER_GET, frame, expected_frame_size);
    frame_generator.add_value(user_unique_id_node, DESIRED_ATTRIBUTE);
    frame_generator.validate_frame(frame_length);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while generating USER_GET frame : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  // This special user ID will contains the unaffected credentials.
  if (user_id == 0) {
    users_interview_state::set_user_0_node(user_unique_id_node);
  }

  return SL_STATUS_OK;
}

// TODO : Update with values in ZW_cmdclass.h
enum class user_report_type_t : uint8_t {
  USER_ADDED                          = 0x00,
  USER_MODIFIED                       = 0x01,
  USER_DELETED                        = 0x02,
  USER_UNCHANGED                      = 0x03,
  RESPONSE_TO_GET                     = 0x04,
  USER_ADD_REJECTED_LOCATION_OCCUPIED = 0x05,
  USER_MODIFY_REJECTED_LOCATION_EMPTY = 0x06,
  NON_ZERO_EXPIRING_MINUTES_INVALID   = 0x07
};

sl_status_t zwave_command_class_user_credential_user_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  sl_log_debug(LOG_TAG, "User Report");

  attribute_store::attribute endpoint_node(
    zwave_command_class_get_endpoint_node(connection_info));

  // Resolve user 0 node if needed
  // We need to do that here so that the attribute store mark this node as resolved
  users_interview_state::resolve_user_0_node();

  const uint8_t expected_min_size = 16;

  try {
    zwave_frame_parser parser(frame_data, frame_length);

    if (!parser.is_frame_size_valid(expected_min_size, UINT8_MAX)) {
      sl_log_error(LOG_TAG, "Invalid frame size for User Report frame");
      return SL_STATUS_FAIL;
    }

    auto user_report_type = static_cast<user_report_type_t>(parser.read_byte());

    auto next_user_id
      = parser.read_sequential<user_credential_user_unique_id_t>(2);

    user_credential_modifier_type_t user_modifier_type = parser.read_byte();
    auto user_modifier_id
      = parser.read_sequential<user_credential_modifier_node_id_t>(2);

    // Get User ID
    auto current_user_id
      = parser.read_sequential<user_credential_user_unique_id_t>(2);

    sl_log_debug(LOG_TAG,
                 "User report for user %d. User report type %d",
                 current_user_id,
                 user_report_type);

    // CC:0083.01.05.11.006: Zero is an invalid User Unique Identifier and MUST NOT be used by the node
    if (current_user_id == 0) {
      if (user_report_type == user_report_type_t::RESPONSE_TO_GET) {
        sl_log_info(LOG_TAG, "No users was found on the device.");
        return SL_STATUS_OK;
      } else if (user_report_type == user_report_type_t::USER_DELETED) {
        sl_log_info(LOG_TAG, "Request to delete all users");

        for (auto user_node:
             endpoint_node.children(ATTRIBUTE(USER_UNIQUE_ID))) {
          // Don't delete special user 0
          if (user_node.reported_exists()
              && user_node.reported<user_credential_user_unique_id_t>() == 0) { //NOSONAR - Don't have a better way to write this
            continue;
          }

          attribute_store_delete_node(user_node);
        }

        return SL_STATUS_OK;
      } else {
        sl_log_error(LOG_TAG,
                     "User report with ID 0 received. This is an invalid User "
                     "Unique Identifier and MUST NOT be used by the node.");
        return SL_STATUS_FAIL;
      }
    }

    // Lambda function to remove user node in an invalid state
    auto remove_current_user_node = [&]() {
      get_user_unique_id_node(endpoint_node, current_user_id, DESIRED_ATTRIBUTE)
        .delete_node();
    };

    // Current user id node that will be used later
    // Each report type has a different behavior
    attribute_store::attribute current_user_id_node;
    switch (user_report_type) {
      // In either case we want to create the user node if it doesn't exists
      // or update the desired value if it does
      case user_report_type_t::USER_ADDED:
      case user_report_type_t::RESPONSE_TO_GET:
        current_user_id_node
          = create_or_update_desired_value(endpoint_node,
                                           ATTRIBUTE(USER_UNIQUE_ID),
                                           current_user_id);
        break;
      // We should have a record of given user ID
      case user_report_type_t::USER_MODIFIED:
      case user_report_type_t::USER_DELETED:
        current_user_id_node = get_user_unique_id_node(endpoint_node,
                                                       current_user_id,
                                                       REPORTED_ATTRIBUTE);
        break;
      // Special/Errors cases
      case user_report_type_t::USER_UNCHANGED:
        sl_log_info(LOG_TAG, "User %d is unchanged", current_user_id);
        return SL_STATUS_OK;
      case user_report_type_t::USER_ADD_REJECTED_LOCATION_OCCUPIED:
        sl_log_warning(LOG_TAG,
                       "User %d was not added since it already exists. Try to "
                       "modify it instead.",
                       current_user_id);
        remove_current_user_node();
        return SL_STATUS_OK;
      case user_report_type_t::USER_MODIFY_REJECTED_LOCATION_EMPTY:
        sl_log_warning(LOG_TAG,
                       "User %d was not modified since it doesn't exists. Try "
                       "to add it instead.",
                       current_user_id);
        remove_current_user_node();
        return SL_STATUS_OK;
      case user_report_type_t::NON_ZERO_EXPIRING_MINUTES_INVALID:
        sl_log_warning(LOG_TAG,
                       "User %d was not modified/added since the expiring "
                       "timeout minutes is invalid.",
                       current_user_id);
        return SL_STATUS_OK;
      default:
        sl_log_error(LOG_TAG, "Invalid value for user report type.");
        return SL_STATUS_FAIL;
    };

    // Deleted special case
    if (user_report_type == user_report_type_t::USER_DELETED) {
      sl_log_info(LOG_TAG, "User %d has been deleted", current_user_id);
      current_user_id_node.delete_node();
      return SL_STATUS_OK;
    }

    // Set already parsed values
    current_user_id_node.emplace_node(ATTRIBUTE(USER_MODIFIER_TYPE))
      .set_reported(user_modifier_type);
    current_user_id_node.emplace_node(ATTRIBUTE(USER_MODIFIER_NODE_ID))
      .set_reported(user_modifier_id);

    // Keep parsing the frame
    parser.read_byte(current_user_id_node.emplace_node(ATTRIBUTE(USER_TYPE)));
    parser.read_byte_with_bitmask(
      {USER_REPORT_PROPERTIES1_USER_ACTIVE_STATE_BIT_MASK,
       current_user_id_node.emplace_node(ATTRIBUTE(USER_ACTIVE_STATE))});
    parser.read_byte(
      current_user_id_node.emplace_node(ATTRIBUTE(CREDENTIAL_RULE)));
    parser.read_sequential<user_credential_expiring_timeout_minutes_t>(
      2,
      current_user_id_node.emplace_node(
        ATTRIBUTE(USER_EXPIRING_TIMEOUT_MINUTES)));
    parser.read_byte_with_bitmask(
      {USER_REPORT_PROPERTIES2_USER_NAME_ENCODING_MASK,
       current_user_id_node.emplace_node(ATTRIBUTE(USER_NAME_ENCODING))});
    parser.read_string(current_user_id_node.emplace_node(ATTRIBUTE(USER_NAME)));

    // Get credentials
    trigger_get_credential(current_user_id_node, 0, 0);

    if (next_user_id != 0
        && user_report_type == user_report_type_t::RESPONSE_TO_GET) {
      if (!user_exists(endpoint_node, next_user_id)) {
        users_interview_state::schedule_next_user_interview(next_user_id);
      } else {
        sl_log_error(LOG_TAG,
                     "User %d already exists. Not discovering more users.",
                     next_user_id);
      }
    } else {
      sl_log_debug(LOG_TAG,
                   "User %d is the last user to be discovered.",
                   current_user_id);
    }
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while parsing User Report frame : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Checksum helpers
/////////////////////////////////////////////////////////////////////////////

/**
 * @brief Compte the checksum and verify the checksum integrity. 
 * 
 * Compare crc16 of checksum_data and expected checksum and put it in the 
 * checksum_error_type if not matching.  
 * 
 * @param base_node The base node to put the error attribute
 * @param checksum_error_type The type of the error attribute
 * @param checksum_data The data to compute the checksum. Checksum will be 0 if empty.
 * @param expected_checksum The expected checksum
 * 
 * @return SL_STATUS_OK if checksum is correct, SL_STATUS_FAIL otherwise
*/
sl_status_t check_checksum_value(attribute_store::attribute base_node,
                                 attribute_store_type_t checksum_error_type,
                                 user_credential_checksum_t computed_checksum,
                                 user_credential_checksum_t expected_checksum)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (computed_checksum != expected_checksum) {
    sl_log_error(LOG_TAG,
                 "Checksum mismatch (%s). Expected 0x%X, got 0x%X",
                 base_node.value_to_string().c_str(),
                 expected_checksum,
                 computed_checksum);
    base_node.emplace_node(checksum_error_type).set_reported(computed_checksum);
  } else {
    // If we don't have any errors we remove the checksum_error_type node
    base_node.child_by_type(checksum_error_type).delete_node();
    status = SL_STATUS_OK;
  }

  return status;
}

/////////////////////////////////////////////////////////////////////////////
// User Checksum Get/Report
/////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_user_credential_user_checksum_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "User Checksum Get");
  attribute_store::attribute checksum_node(node);

  auto user_id_node = checksum_node.first_parent(ATTRIBUTE(USER_UNIQUE_ID));

  if (!user_id_node.is_valid()) {
    sl_log_error(
      LOG_TAG,
      "Can't find User Unique ID node. Not sending User Checksum Get.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  constexpr uint8_t expected_frame_size
    = static_cast<uint8_t>(sizeof(ZW_USER_CHECKSUM_GET_FRAME));

  try {
    frame_generator.initialize_frame(USER_CHECKSUM_GET,
                                     frame,
                                     expected_frame_size);
    frame_generator.add_value(user_id_node);
    frame_generator.validate_frame(frame_length);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while generating User Checksum Get frame : %s",
                 e.what());
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_user_checksum_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  sl_status_t result = SL_STATUS_FAIL;

  constexpr uint8_t expected_frame_length
    = sizeof(ZW_USER_CHECKSUM_REPORT_FRAME);

  attribute_store::attribute endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  try {
    zwave_frame_parser parser(frame_data, frame_length);

    if (!parser.is_frame_size_valid(expected_frame_length)) {
      sl_log_error(LOG_TAG,
                   "Invalid frame size for User Checksum Report frame");
      return SL_STATUS_NOT_SUPPORTED;
    }

    // Parse the frame
    const auto user_id
      = parser.read_sequential<user_credential_user_unique_id_t>(2);
    const auto user_checksum
      = parser.read_sequential<user_credential_checksum_t>(2);

    sl_log_debug(LOG_TAG,
                 "User Checksum Report. Source User ID: %d / "
                 "Checksum: 0x%X",
                 user_id,
                 user_checksum);

    auto user_node
      = get_user_unique_id_node(endpoint_node, user_id, REPORTED_ATTRIBUTE);

    // Compute checksum ourselves to see if it matches
    user_credential::checksum_calculator checksum_calculator;
    add_user_node_to_checksum(user_node, checksum_calculator);

    result = check_checksum_value(user_node,
                                  ATTRIBUTE(USER_CHECKSUM_MISMATCH_ERROR),
                                  checksum_calculator.compute_checksum(),
                                  user_checksum);

    user_node.emplace_node(ATTRIBUTE(USER_CHECKSUM))
      .set_reported(user_checksum);

    if (result == SL_STATUS_OK) {
      send_message_to_mqtt(SL_LOG_INFO,
                           "User Checksum for user " + std::to_string(user_id)
                             + " is correct.");
    } else {
      send_message_to_mqtt(SL_LOG_ERROR,
                           "Mismatch User Checksum for user "
                             + std::to_string(user_id));
    }
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while parsing User Checksum Report frame : %s",
                 e.what());
    return SL_STATUS_NOT_SUPPORTED;
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////////
// Credential Checksum Get/Report
/////////////////////////////////////////////////////////////////////////////

static sl_status_t zwave_command_class_user_credential_credential_checksum_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "Credential Checksum Get");

  attribute_store::attribute credential_checksum_node(node);
  try {
    auto credential_type_node = credential_checksum_node.first_parent(
      ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE));

    if (!credential_type_node.is_valid()) {
      sl_log_error(LOG_TAG,
                   "Can't find Credential Type node. Not sending Credential "
                   "Checksum Get.");
      return SL_STATUS_NOT_SUPPORTED;
    }

    frame_generator.initialize_frame(CREDENTIAL_CHECKSUM_GET,
                                     frame,
                                     sizeof(ZW_CREDENTIAL_CHECKSUM_GET_FRAME));

    frame_generator.add_value(credential_type_node, REPORTED_ATTRIBUTE);
    frame_generator.validate_frame(frame_length);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while generating Credential Checksum Get frame : %s",
                 e.what());
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

sl_status_t
  zwave_command_class_user_credential_credential_checksum_handle_report(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  sl_status_t result = SL_STATUS_FAIL;

  attribute_store::attribute endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  constexpr uint8_t expected_frame_length
    = sizeof(ZW_CREDENTIAL_CHECKSUM_REPORT_FRAME);
  try {
    zwave_frame_parser parser(frame_data, frame_length);

    if (!parser.is_frame_size_valid(expected_frame_length)) {
      sl_log_error(LOG_TAG,
                   "Invalid frame size for Credential Checksum Report frame");
      return SL_STATUS_NOT_SUPPORTED;
    }

    // Parse the frame
    const user_credential_type_t credential_type = parser.read_byte();
    const auto credential_checksum
      = parser.read_sequential<user_credential_checksum_t>(2);

    sl_log_debug(LOG_TAG,
                 "Credential Checksum Report. Credential type: %d / "
                 "Checksum: 0x%X",
                 credential_type,
                 credential_checksum);

    // Get the credential type node
    auto credential_type_node = endpoint_node.child_by_type_and_value(
      ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE),
      credential_type);
    if (!credential_type_node.is_valid()) {
      sl_log_error(LOG_TAG,
                   "Can't find Credential Type %s reported by Credential "
                   "Checksum Report",
                   get_credential_type_debug_str(credential_type).c_str());
      return SL_STATUS_FAIL;
    }

    credential_type_node.emplace_node(ATTRIBUTE(CREDENTIAL_CHECKSUM))
      .set_reported(credential_checksum);

    // Compute checksum ourselves to see if it matches
    user_credential::checksum_calculator checksum_calculator;
    for_each_credential_type_nodes(
      endpoint_node,
      [&checksum_calculator](auto current_credential_type_node) {
        for (auto current_credential_slot_node:
             current_credential_type_node.children(
               ATTRIBUTE(CREDENTIAL_SLOT))) {
          if (!current_credential_slot_node.reported_exists()) {
            sl_log_debug(
              LOG_TAG,
              "%s reported value is not defined. Not adding to checksum.",
              current_credential_slot_node.value_to_string());
            continue;
          }

          checksum_calculator.add_node(current_credential_slot_node);
          checksum_calculator.add_node(
            current_credential_slot_node.child_by_type(
              ATTRIBUTE(CREDENTIAL_DATA)));
        }
      },
      credential_type);

    result = check_checksum_value(credential_type_node,
                                  ATTRIBUTE(CREDENTIAL_CHECKSUM_MISMATCH_ERROR),
                                  checksum_calculator.compute_checksum(),
                                  credential_checksum);

    if (result == SL_STATUS_OK) {
      send_message_to_mqtt(SL_LOG_INFO,
                           "Credential Checksum for "
                             + cred_type_get_enum_value_name(credential_type)
                             + " is correct.");
    } else {
      send_message_to_mqtt(SL_LOG_ERROR,
                           "Mismatch Credential Checksum for "
                             + cred_type_get_enum_value_name(credential_type));
    }
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while parsing Credential Checksum Report frame : %s",
                 e.what());
    return SL_STATUS_NOT_SUPPORTED;
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////////
// Admin code Set/Get/Report
/////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_user_credential_admin_pin_code_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "Admin PIN Code Set");

  try {
    // Node setup
    attribute_store::attribute admin_pin_code_node(node);

    // Get Values
    std::vector<uint8_t> admin_pin_code
      = admin_pin_code_node.desired<std::vector<uint8_t>>();

    // This represents the case where the admin pin code is empty
    // We need this workaround because attribute store doesn't support empty values as
    // they are treated as non existing
    if (is_admin_code_empty(admin_pin_code)) {
      admin_pin_code.clear();
    }

    user_credential::credential_capabilities credential_capabilities(
      admin_pin_code_node.first_parent(ATTRIBUTE_ENDPOINT_ID));

    if (admin_pin_code.empty()
        && !credential_capabilities.is_admin_code_deactivation_supported()) {
      send_message_to_mqtt(SL_LOG_ERROR,
                           "Admin PIN code is empty and the device doesn't "
                           "support deactivation. Not sending Admin PIN code.");
      return SL_STATUS_NOT_SUPPORTED;
    }

    // 2 for header
    // 1 for length
    const uint8_t admin_code_size = static_cast<uint8_t>(admin_pin_code.size());
    const uint8_t expected_frame_size = admin_code_size + 3;

    // Creating the frame
    frame_generator.initialize_frame(ADMIN_PIN_CODE_SET,
                                     frame,
                                     expected_frame_size);

    // In every case we add 0 to the frame
    frame_generator.add_raw_byte(admin_code_size);

    // Do not add anything to the frame if the admin pin code is empty
    if (!admin_pin_code.empty()) {
      frame_generator.add_value(admin_pin_code_node,
                                DESIRED_OR_REPORTED_ATTRIBUTE);
    }

    frame_generator.validate_frame(frame_length);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while generating Admin PIN Code frame : %s",
                 e.what());
    return SL_STATUS_NOT_SUPPORTED;
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_user_credential_admin_pin_code_get(
  [[maybe_unused]] attribute_store_node_t node,
  uint8_t *frame,
  uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "Admin PIN Code Get");
  // No definition yet for ADMIN_PIN_CODE_GET we use raw value
  return frame_generator.generate_no_args_frame(ADMIN_PIN_CODE_GET,
                                                frame,
                                                frame_length);
}

sl_status_t zwave_command_class_user_credential_admin_pin_code_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  sl_log_debug(LOG_TAG, "Admin PIN Code Report");

  attribute_store::attribute endpoint_node(
    zwave_command_class_get_endpoint_node(connection_info));

  const uint8_t min_size = 3;

  try {
    zwave_frame_parser parser(frame_data, frame_length);
    // 0xF since the size of the Admin PIN code is on 4 bits
    if (!parser.is_frame_size_valid(min_size, min_size + 0xF)) {
      sl_log_error(LOG_TAG,
                   "Invalid frame size for Admin PIN Code Report frame");
      return SL_STATUS_FAIL;
    }

    // Parse the frame
    constexpr uint8_t admin_code_op_result_bitmask = 0b11110000;
    constexpr uint8_t admin_code_length_bitmask    = 0b00001111;
    // First parse the code size and operation result
    auto result = parser.read_byte_with_bitmask(
      {{admin_code_op_result_bitmask,
        endpoint_node.emplace_node(ATTRIBUTE(ADMIN_PIN_CODE_OPERATION_RESULT))},
       {admin_code_length_bitmask}});

    auto admin_code_op_result = result[admin_code_op_result_bitmask];
    switch (admin_code_op_result) {
      case ADMIN_PIN_CODE_REPORT_MODIFIED:  // Modified
        break;
      case ADMIN_PIN_CODE_REPORT_UNMODIFIED:  // Unmodified
        send_message_to_mqtt(SL_LOG_INFO, "Admin PIN code is unchanged.");
        break;
      case ADMIN_PIN_CODE_REPORT_RESPONSE_TO_GET:  // Response to get
        break;
      case ADMIN_PIN_CODE_REPORT_DUPLICATE_CREDENTIAL:  // Duplicate Credential
        send_message_to_mqtt(SL_LOG_ERROR,
                             "Admin Code was not modified - a duplicate PIN "
                             "code exists in the Credential database.");
        break;
      case ADMIN_PIN_CODE_REPORT_MANUFACTURER_SECURITY_RULE:  // Manufacturer Security Rule
        send_message_to_mqtt(SL_LOG_ERROR,
                             "Admin Code was not modified as the provided code "
                             "fails a manufacturer security rule");
        break;
      case ADMIN_PIN_CODE_REPORT_NOT_SUPPORTED:  // Admin Code not supported
        send_message_to_mqtt(SL_LOG_ERROR,
                             "Admin Code was not modified as the device does "
                             "not support Admin Code functionality.");
        break;
      case ADMIN_PIN_CODE_REPORT_DEACTIVATION_NOT_SUPPORTED:  // Admin Code Deactivation not supported
        send_message_to_mqtt(
          SL_LOG_ERROR,
          "Admin Code was not modified as the device does "
          "not support Admin Code deactivation functionality.");
        break;
      case ADMIN_PIN_CODE_REPORT_UNSPECIFIED_ERROR:  // Unspecified Node Error
        send_message_to_mqtt(
          SL_LOG_ERROR,
          "Unknown issue with Get/Set Admin Code operation.");
        break;
      default:
        send_message_to_mqtt(SL_LOG_ERROR,
                             "Admin PIN Code unknown Operation Result.");
        return SL_STATUS_FAIL;
    }

    // Modified or Response to get
    if (admin_code_op_result == ADMIN_PIN_CODE_REPORT_RESPONSE_TO_GET
        || admin_code_op_result == ADMIN_PIN_CODE_REPORT_MODIFIED) {
      auto admin_code_size = result[admin_code_length_bitmask];
      sl_log_debug(LOG_TAG, "Admin PIN Code size : %d", admin_code_size);

      auto admin_pin_code_node
        = endpoint_node.emplace_node(ATTRIBUTE(ADMIN_PIN_CODE));

      if (admin_code_size != 0) {
        parser.read_sequential<std::vector<uint8_t>>(admin_code_size,
                                                     admin_pin_code_node);
      } else {
        // Since set an empty vector is not supported we delete the node
        sl_log_debug(LOG_TAG, "Admin PIN Code is empty.");
        set_empty_admin_code(admin_pin_code_node, REPORTED_ATTRIBUTE);
      }
    }

    return SL_STATUS_OK;
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while parsing Admin Pin Report frame : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Post interview actions
/////////////////////////////////////////////////////////////////////////////
void zwave_network_status_changed(attribute_store_node_t node,
                                  attribute_store_change_t change)
{
  // We englobe this in case we can't read an attribute or something goes wrong
  try {
    attribute_store::attribute network_status_node(node);
    auto node_id_node = network_status_node.first_parent(ATTRIBUTE_NODE_ID);

    // Should not happen but we check anyway to prevent an exception to raise
    if (!node_id_node.reported_exists()) {
      return;
    }

    auto node_id = node_id_node.reported<zwave_node_id_t>();

    // If we are updating the zpc node or if we trying to delete the attribute we don't want to do anything
    if (change != ATTRIBUTE_UPDATED || get_zpc_node_id_node() == node_id_node) {
      return;
    }

    // In case we are not ready yet and don't have a reported value
    if (!network_status_node.reported_exists()) {
      return;
    }

    // If the endpoint report is marked as ONLINE_FUNCTIONAL
    if (network_status_node.reported<NodeStateNetworkStatus>()
        == ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL) {
      sl_log_debug(LOG_TAG,
                   "Node %d is now ONLINE_FUNCTIONAL : start the delayed "
                   "interview process",
                   node_id);
      // Perform action on each endpoint that supports User Credential Command class
      sl_log_debug(LOG_TAG, "Checking endpoints...");
      for (auto endpoint_node: node_id_node.children(ATTRIBUTE_ENDPOINT_ID)) {
        auto endpoint_id = endpoint_node.reported<zwave_endpoint_id_t>();
        // Check if the endpoint supports User Credential Command class
        if (!zwave_node_supports_command_class(COMMAND_CLASS_USER_CREDENTIAL,
                                               node_id,
                                               endpoint_id)) {
          continue;
        }
        
        sl_log_debug(LOG_TAG,
                     "Endpoint %d supports User Credential.",
                     endpoint_id);
        auto user_count
          = endpoint_node.children(ATTRIBUTE(USER_UNIQUE_ID)).size();
        if (user_count != 0) {
          sl_log_debug(LOG_TAG, "Users already discovered. No actions needed.");
          continue;
        }

        sl_log_debug(LOG_TAG,
                     "No user found. Starting User and Credential interview");
        // Start the interview process with user ID = 0
        endpoint_node.add_node(ATTRIBUTE(USER_UNIQUE_ID))
          .set_desired<user_credential_user_unique_id_t>(0);
      }
      sl_log_debug(LOG_TAG, "Done.");
    }
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while handling network status change : %s",
                 e.what());
  }
}

/////////////////////////////////////////////////////////////////////////////
// Class logic
/////////////////////////////////////////////////////////////////////////////

// Control handler
sl_status_t zwave_command_class_user_credential_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case USER_CAPABILITIES_REPORT:
      return zwave_command_class_user_credential_user_capabilities_handle_report(
        connection_info,
        frame_data,
        frame_length);
    case CREDENTIAL_CAPABILITIES_REPORT:
      return zwave_command_class_user_credential_credential_capabilities_handle_report(
        connection_info,
        frame_data,
        frame_length);
    case ALL_USERS_CHECKSUM_REPORT:
      return zwave_command_class_user_credential_all_user_checksum_handle_report(
        connection_info,
        frame_data,
        frame_length);
    case USER_REPORT:
      return zwave_command_class_user_credential_user_handle_report(
        connection_info,
        frame_data,
        frame_length);
    case CREDENTIAL_REPORT:
      return zwave_command_class_user_credential_credential_handle_report(
        connection_info,
        frame_data,
        frame_length);
    case CREDENTIAL_LEARN_REPORT:
      return zwave_command_class_user_credential_credential_learn_status_report(
        connection_info,
        frame_data,
        frame_length);
    case USER_CREDENTIAL_ASSOCIATION_REPORT:
      return zwave_command_class_user_credential_uuic_association_report(
        connection_info,
        frame_data,
        frame_length);
    case USER_CHECKSUM_REPORT:
      return zwave_command_class_user_credential_user_checksum_handle_report(
        connection_info,
        frame_data,
        frame_length);
    case CREDENTIAL_CHECKSUM_REPORT:
      return zwave_command_class_user_credential_credential_checksum_handle_report(
        connection_info,
        frame_data,
        frame_length);
    case 0x1C:  // ADMIN_PIN_CODE_REPORT
      return zwave_command_class_user_credential_admin_pin_code_handle_report(
        connection_info,
        frame_data,
        frame_length);
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

// Entry point
sl_status_t zwave_command_class_user_credential_init()
{
  attribute_store_register_callback_by_type(
    &zwave_command_class_user_credential_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  attribute_resolver_register_rule(
    ATTRIBUTE(NUMBER_OF_USERS),
    NULL,
    &zwave_command_class_user_credential_user_capabilities_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(SUPPORT_CREDENTIAL_CHECKSUM),
    NULL,
    &zwave_command_class_user_credential_credential_capabilities_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(ALL_USERS_CHECKSUM),
    NULL,
    &zwave_command_class_user_credential_all_user_checksum_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(USER_UNIQUE_ID),
    NULL,
    &zwave_command_class_user_credential_user_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(USER_OPERATION_TYPE),
    &zwave_command_class_user_credential_user_set,
    NULL);

  attribute_resolver_register_rule(
    ATTRIBUTE(CREDENTIAL_SLOT),
    NULL,
    &zwave_command_class_user_credential_credential_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(CREDENTIAL_OPERATION_TYPE),
    &zwave_command_class_user_credential_credential_set,
    NULL);

  attribute_resolver_register_rule(
    ATTRIBUTE(CREDENTIAL_LEARN_OPERATION_TYPE),
    &zwave_command_class_user_credential_credential_learn_start,
    NULL);

  attribute_resolver_register_rule(
    ATTRIBUTE(CREDENTIAL_LEARN_STOP),
    &zwave_command_class_user_credential_credential_learn_cancel,
    NULL);

  attribute_resolver_register_rule(
    ATTRIBUTE(ASSOCIATION_DESTINATION_CREDENTIAL_SLOT),
    &zwave_command_class_user_credential_uuic_association_set,
    NULL);

  attribute_resolver_register_rule(
    ATTRIBUTE(USER_CHECKSUM),
    NULL,
    &zwave_command_class_user_credential_user_checksum_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(CREDENTIAL_CHECKSUM),
    NULL,
    &zwave_command_class_user_credential_credential_checksum_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(ADMIN_PIN_CODE),
    &zwave_command_class_user_credential_admin_pin_code_set,
    &zwave_command_class_user_credential_admin_pin_code_get);

  // https://github.com/Z-Wave-Alliance/AWG/pull/124#discussion_r1484473752
  // Discussion about delaying the user interview process after the inclusion

  // Proposed Unify-way to delay users get AFTER interview process
  attribute_store_register_callback_by_type(
    &zwave_network_status_changed,
    DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS);

  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler = zwave_command_class_user_credential_control_handler;
  // CHECKME : Is this right ?
  handler.minimal_scheme = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_USER_CREDENTIAL;
  handler.version                    = 1;
  handler.command_class_name         = "User Credential";
  handler.comments                   = "Experimental";

  return zwave_command_handler_register_handler(handler);
}