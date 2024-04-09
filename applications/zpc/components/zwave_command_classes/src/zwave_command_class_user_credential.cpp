
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
#include "zwave_command_class_user_credential_types.h"
#include "zwave_command_classes_utils.h"
#include "ZW_classcmd.h"

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_command_handler.h"
#include "zwave_command_class_version_types.h"
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store.h"

// Unify
#include "attribute_resolver.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_type_registration.h"
#include "sl_log.h"

// DotDot
#include "unify_dotdot_attribute_store_node_state.h"

// Cpp related
#include <vector>
#include <string>

// Macro
#define LOG_TAG         "zwave_command_class_user_credential"
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_##type
// Used to get user names
// If the user name has a size > to this number, we will truncate it
// Specification says that payload data should not exceeded 64 bytes.
#define MAX_CHAR_SIZE 64

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

/////////////////////////////////////////////////////////////////////////////
// Data struct
/////////////////////////////////////////////////////////////////////////////

struct uint16_exploded {
  uint8_t msb;  // Most Significant Bit
  uint8_t lsb;  // Less Significant Bit
};

// Used to define reported values
struct user_field_data {
  attribute_store_type_t node_type;
  uint8_t report_index;
  uint8_t bitmask     = 0;
  uint8_t shift_right = 0;
};

// Used to create command frame
struct attribute_command_data {
  // Attribute type that will be fetched from the base_node
  attribute_store_type_t attribute_type;
  // Attribute value state (reported, desired,...)
  attribute_store_node_value_state_t attribute_state;
  // If not ATTRIBUTE_STORE_INVALID_NODE, the function will not fetch attribute_type
  // but will use this node directly
  attribute_store_node_t node = ATTRIBUTE_STORE_INVALID_NODE;
};

/////////////////////////////////////////////////////////////////////////////
// Type Helpers
/////////////////////////////////////////////////////////////////////////////
uint16_t get_uint16_value(const uint8_t *frame, uint16_t start_index)
{
  uint16_t extracted_value = 0;
  for (int i = 0; i < 2; i++) {
    extracted_value = (extracted_value << 8) | frame[start_index + i];
  }

  return extracted_value;
}

// Transform a uint16_t into 2 uint8_t
uint16_exploded explode_uint16(uint16_t value)
{
  uint8_t msb = (value & 0xFF00) >> 8;
  uint8_t lsb = (value & 0x00FF);
  return uint16_exploded {msb, lsb};
}

std::string get_string_value(const uint8_t *frame_data,
                             uint16_t start_index,
                             uint8_t str_size)
{
  std::string value;
  // Check if our name fits our buffer ; if not it is truncated
  if (str_size > MAX_CHAR_SIZE) {
    sl_log_warning(LOG_TAG, "Invalid char size");
    str_size = MAX_CHAR_SIZE;
  }

  for (int i = 0; i < str_size; i++) {
    value += frame_data[start_index + i];
  }

  return value;
}

/////////////////////////////////////////////////////////////////////////////
// Command Class Helper
/////////////////////////////////////////////////////////////////////////////

/**
 * @brief Get user id node 
 * 
 * @warning state can't be DESIRED_OR_REPORTED_ATTRIBUTE or it will not work
 * 
 * @param endpoint_node  Endpoint point node
 * @param user_id        User ID to find 
 * @param state          Check reported or desired value. 
 * @param user_id_node   User id node will be stored here if found
 * 
 * @return true  User id exists
 * @return false User id doesn't exists
 */
bool get_user_id_node(attribute_store_node_t endpoint_node,
                      user_credential_user_unique_id_t user_id,
                      attribute_store_node_value_state_t state,
                      attribute_store_node_t &user_id_node)
{
  user_id_node
    = attribute_store_get_node_child_by_value(endpoint_node,
                                              ATTRIBUTE(USER_UNIQUE_ID),
                                              state,
                                              (uint8_t *)&user_id,
                                              sizeof(user_id),
                                              0);

  return attribute_store_node_exists(user_id_node);
}

/**
 * @brief Get node associated with user ID (reported)
 * 
 * @warning This function only checks the reported User Unique
 * 
 * @param endpoint_node  Current endpoint node 
 * @param user_id        User ID  
 * 
 * @return attribute_store_node_t If User ID exists
 * @return INVALID_ATTRIBUTE_STORE_NODE If User ID does not exist
 */
attribute_store_node_t
  get_reported_user_id_node(attribute_store_node_t endpoint_node,
                            user_credential_user_unique_id_t user_id)
{
  attribute_store_node_t user_id_node;
  get_user_id_node(endpoint_node, user_id, REPORTED_ATTRIBUTE, user_id_node);
  return user_id_node;
}

/**
 * @brief Get node associated with user ID (desired)
 * 
 * @warning This function only checks the reported User Unique
 * 
 * @param endpoint_node  Current endpoint node 
 * @param user_id        User ID  
 * 
 * @return attribute_store_node_t If User ID exists
 * @return INVALID_ATTRIBUTE_STORE_NODE If User ID does not exist
 */
attribute_store_node_t
  get_desired_user_id_node(attribute_store_node_t endpoint_node,
                            user_credential_user_unique_id_t user_id)
{
  attribute_store_node_t user_id_node;
  get_user_id_node(endpoint_node, user_id, DESIRED_ATTRIBUTE, user_id_node);
  return user_id_node;
}

/**
 * @brief Get credential node associated with credential_type and user_id.
 * 
 * @param endpoint_node         Current endpoint node
 * @param user_id               User ID
 * @param credential_type       Credential type
 * @param state                 Check reported or desired value. 
 * @param credential_type_node  Credential node will be stored here if found
 * 
 * @return true  Credential Type exists
 * @return false Credential Type doesn't exists
   */
bool get_credential_type_node(attribute_store_node_t endpoint_node,
                              user_credential_user_unique_id_t user_id,
                              user_credential_type_t credential_type,
                              attribute_store_node_value_state_t state,
                              attribute_store_node_t &credential_type_node)
{
  attribute_store_node_t user_id_node
    = get_reported_user_id_node(endpoint_node, user_id);

  credential_type_node
    = attribute_store_get_node_child_by_value(user_id_node,
                                              ATTRIBUTE(CREDENTIAL_TYPE),
                                              state,
                                              (uint8_t *)&credential_type,
                                              sizeof(credential_type),
                                              0);

  return attribute_store_node_exists(credential_type_node);
}

/**
 * @brief Get credential slot node
 * 
 * @warning state can't be DESIRED_OR_REPORTED_ATTRIBUTE or it will not work
 * 
 * @param credential_type_node  Endpoint point node
 * @param credential_slot       Credential Slot to find 
 * @param state                 Check reported or desired value. 
 * @param credential_slot_node  Credential Slot node will be stored here if found
 * 
 * @return true  Credential Slot exists
 * @return false Credential Slot doesn't exists
 */
bool get_credential_slot_node(attribute_store_node_t credential_type_node,
                              user_credential_slot_t credential_slot,
                              attribute_store_node_value_state_t state,
                              attribute_store_node_t &credential_slot_node)
{
  credential_slot_node
    = attribute_store_get_node_child_by_value(credential_type_node,
                                              ATTRIBUTE(CREDENTIAL_SLOT),
                                              state,
                                              (uint8_t *)&credential_slot,
                                              sizeof(credential_slot),
                                              0);

  return attribute_store_node_exists(credential_slot_node);
}

/////////////////////////////////////////////////////////////////////////////
// Attributes helpers
/////////////////////////////////////////////////////////////////////////////

/** @brief Set reported attributes based on user_data
 * 
 * @note This function also undefine all desired values
 * 
 * @param base_node  Parent node of the newly created attributes
 * @param frame_data Frame data to interpret
 * @param user_data  User data to interpret frame_data
 * 
 * @return sl_status_t SL_STATUS_OK if everything was fine
*/
sl_status_t
  set_reported_attributes(attribute_store_node_t base_node,
                          const uint8_t *frame_data,
                          const std::vector<user_field_data> &user_data)
{
  sl_status_t status = SL_STATUS_OK;

  for (const auto &field: user_data) {
    attribute_store_storage_type_t storage_type
      = attribute_store_get_storage_type(field.node_type);

    switch (storage_type) {
      case U8_STORAGE_TYPE: {
        uint8_t uint8_value = frame_data[field.report_index];
        if (field.bitmask != 0) {
          uint8_value = (uint8_value & field.bitmask) >> field.shift_right;
        }
        status |= attribute_store_set_child_reported(base_node,
                                                     field.node_type,
                                                     &uint8_value,
                                                     sizeof(uint8_value));
      } break;
      // Unsigned 16-bit integers are used for this attribute
      case U16_STORAGE_TYPE: {
        uint16_t uint16_value
          = get_uint16_value(frame_data, field.report_index);
        status |= attribute_store_set_child_reported(base_node,
                                                     field.node_type,
                                                     &uint16_value,
                                                     sizeof(uint16_value));

        break;
      }
      default:
        sl_log_error(LOG_TAG,
                     "Internal error : unsupported storage_type in "
                     "set_reported_attributes");
        return SL_STATUS_NOT_SUPPORTED;
    }

    // Undefined desired value
    status
      |= attribute_store_set_child_desired(base_node, field.node_type, NULL, 0);
  }

  return status;
}

/**
 * @brief Create a command frame (SET or GET) based on the attribute store
 * 
 * @param command       Command to send (will be in frame[1], e.g USER_SET)
 * @param command_data  Attributes that will be in the frame (in order of appearance in the frame)       
 * @param base_node     If not specified otherwise will fetch the attributes that are under this node
 * @param frame         Frame object from the callback
 * @param frame_length  Frame size from the callback
 * 
 * @return sl_status_t SL_STATUS_OK if everything was fine
 */
sl_status_t
  create_command_frame(uint8_t command,
                       std::vector<attribute_command_data> command_data,
                       attribute_store_node_t base_node,
                       uint8_t *frame,
                       uint16_t *frame_length)
{
  frame[0] = COMMAND_CLASS_USER_CREDENTIAL;
  frame[1] = command;

  uint16_t current_index = 2;

  for (auto &attribute_info: command_data) {
    auto node_storage_type
      = attribute_store_get_storage_type(attribute_info.attribute_type);
    auto attribute_description
      = attribute_store_get_type_name(attribute_info.attribute_type);

    attribute_store_node_t node;
    if (attribute_info.node == ATTRIBUTE_STORE_INVALID_NODE) {
      node = attribute_store_get_first_child_by_type(
        base_node,
        attribute_info.attribute_type);
    } else {
      node = attribute_info.node;
    }

    if (node == ATTRIBUTE_STORE_INVALID_NODE) {
      sl_log_critical(LOG_TAG,
                      "Can't find node for Attribute %s",
                      attribute_description);
      return SL_STATUS_FAIL;
    }

    sl_status_t status;
    switch (node_storage_type) {
      case U8_STORAGE_TYPE: {
        uint8_t uint8_value;
        status                 = attribute_store_read_value(node,
                                            attribute_info.attribute_state,
                                            &uint8_value,
                                            sizeof(uint8_value));
        frame[current_index++] = uint8_value;
      } break;
      case U16_STORAGE_TYPE: {
        uint16_t uint16_value;
        status                 = attribute_store_read_value(node,
                                            attribute_info.attribute_state,
                                            &uint16_value,
                                            sizeof(uint16_value));
        auto exploded_uint16   = explode_uint16(uint16_value);
        frame[current_index++] = exploded_uint16.msb;
        frame[current_index++] = exploded_uint16.lsb;
      } break;
      // Variable length field
      case BYTE_ARRAY_STORAGE_TYPE: {
        // First get the length
        auto credential_length_node = attribute_store_get_node_parent(node);

        uint8_t credential_data_length = 0;
        status = attribute_store_read_value(credential_length_node,
                                            attribute_info.attribute_state,
                                            &credential_data_length,
                                            sizeof(credential_data_length));

        if (status != SL_STATUS_OK) {
          sl_log_error(
            LOG_TAG,
            "Missing BYTE_ARRAY_STORAGE_TYPE length for attribute %s",
            attribute_description);
          return SL_STATUS_NOT_SUPPORTED;
        }

        frame[current_index++] = credential_data_length;

        // Then the data
        std::vector<uint8_t> credential_data;
        credential_data.resize(credential_data_length);
        status = attribute_store_read_value(node,
                                            attribute_info.attribute_state,
                                            credential_data.data(),
                                            credential_data_length);

        for (const uint8_t &cred: credential_data) {
          frame[current_index++] = cred;
        }

      } break;

      case C_STRING_STORAGE_TYPE: {
        char c_user_name[MAX_CHAR_SIZE];
        // Unfortunately attribute_store_get_string is not exposed so we need to do this
        switch (attribute_info.attribute_state) {
          case DESIRED_OR_REPORTED_ATTRIBUTE:
            status
              = attribute_store_get_desired_else_reported_string(node,
                                                                 c_user_name,
                                                                 MAX_CHAR_SIZE);
            break;
          case DESIRED_ATTRIBUTE:
            status = attribute_store_get_desired_string(node,
                                                        c_user_name,
                                                        MAX_CHAR_SIZE);
            break;
          case REPORTED_ATTRIBUTE:
            status = attribute_store_get_reported_string(node,
                                                         c_user_name,
                                                         MAX_CHAR_SIZE);
            break;
        }

        std::string user_name  = c_user_name;
        frame[current_index++] = user_name.length();
        for (const char &c: user_name) {
          frame[current_index++] = c;
        }

      } break;
      default:
        sl_log_critical(LOG_TAG,
                        "Not supported type for %s",
                        attribute_description);
        return SL_STATUS_FAIL;
    }

    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Can't get value of Attribute %s",
                   attribute_description);
      return SL_STATUS_NOT_SUPPORTED;
    }
  }

  *frame_length = current_index;

  return SL_STATUS_OK;
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

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  // The order of the attribute matter since it defines the order of the
  // Z-Wave get command order.
  const attribute_store_type_t attributes[] = {
    ATTRIBUTE(NUMBER_OF_USERS),
    ATTRIBUTE(SUPPORT_CREDENTIAL_CHECKSUM),
  };

  attribute_store_add_if_missing(endpoint_node,
                                 attributes,
                                 COUNT_OF(attributes));
}

/////////////////////////////////////////////////////////////////////////////
// User Credential User Capabilities Get/Report
/////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_user_credential_user_capabilities_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "User Capabilities Get");

  (void)node;  // unused.
  ZW_USER_CAPABILITIES_GET_FRAME *get_frame
    = (ZW_USER_CAPABILITIES_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_USER_CREDENTIAL;
  get_frame->cmd      = USER_CAPABILITIES_GET;
  *frame_length       = sizeof(ZW_USER_CAPABILITIES_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_user_capabilities_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  constexpr uint8_t INDEX_USER_COUNT       = 2;
  constexpr uint8_t INDEX_CREDENTIAL_RULES = 4;
  constexpr uint8_t INDEX_MAX_LENGTH       = 5;
  constexpr uint8_t INDEX_SUPPORT_BITS     = 6;
  constexpr uint8_t INDEX_BITMASK_LENGTH   = 7;

  if (frame_length < INDEX_BITMASK_LENGTH) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  sl_log_debug(LOG_TAG, "User Capabilities Report");

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  uint16_t user_count = get_uint16_value(frame_data, INDEX_USER_COUNT);

  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(NUMBER_OF_USERS),
                                     &user_count,
                                     sizeof(user_count));

  user_credential_supported_credential_rules_t credential_rules
    = frame_data[INDEX_CREDENTIAL_RULES];
  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(SUPPORTED_CREDENTIAL_RULES),
                                     &credential_rules,
                                     sizeof(credential_rules));

  uint8_t max_length = frame_data[INDEX_MAX_LENGTH];
  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(MAX_USERNAME_LENGTH),
                                     &max_length,
                                     sizeof(max_length));

  // Support bits
  uint8_t support_bits = frame_data[INDEX_SUPPORT_BITS];
  // Create a lambda function that do what we want for all of the 3 parameters
  auto emplace_bool_value = [&support_bits,
                             &endpoint_node](uint8_t bitmask,
                                             attribute_store_type_t type) {
    uint8_t bool_value = (support_bits & bitmask) > 0;

    attribute_store_set_child_reported(endpoint_node,
                                       type,
                                       &bool_value,
                                       sizeof(bool_value));

    // CL:0083.XX.XX.XX.X : If the All Users Checksum Support field is set in the User Capabilities Report Command, the controlling
    // node SHOULD send an All Users Checksum Get Command to check if there are existing Users or
    // Credentials present on the supporting node.
    if (type == ATTRIBUTE(SUPPORT_ALL_USERS_CHECKSUM) && bool_value) {
      sl_log_debug(LOG_TAG,
                   "SUPPORT_ALL_USERS_CHECKSUM is set, sending All Users "
                   "Checksum Get Command");
      const attribute_store_type_t attributes[] = {
        ATTRIBUTE(ALL_USERS_CHECKSUM),
      };
      // Trigger get ALL_USERS_CHECKSUM
      attribute_store_add_if_missing(endpoint_node,
                                     attributes,
                                     COUNT_OF(attributes));
    }
  };

  emplace_bool_value(
    USER_CAPABILITIES_REPORT_PROPERTIES1_USER_CHECKSUM_SUPPORT_BIT_MASK,
    ATTRIBUTE(SUPPORT_USER_SCHEDULE));
  emplace_bool_value(
    USER_CAPABILITIES_REPORT_PROPERTIES1_ALL_USERS_CHECKSUM_SUPPORT_BIT_MASK,
    ATTRIBUTE(SUPPORT_ALL_USERS_CHECKSUM));
  emplace_bool_value(
    USER_CAPABILITIES_REPORT_PROPERTIES1_USER_CHECKSUM_SUPPORT_BIT_MASK,
    ATTRIBUTE(SUPPORT_USER_CHECKSUM));

  // Bit mask support
  user_credential_supported_user_type_bitmask_t bitmask = 0x0000;
  uint8_t bitmask_length = frame_data[INDEX_BITMASK_LENGTH];
  // Since we are using uint32_t we can't have more that 4 bit mask
  if (bitmask_length > 4) {
    sl_log_error(LOG_TAG,
                 "user_credential_supported_user_type_bitmask_t length is not "
                 "supported\n");
    return SL_STATUS_NOT_SUPPORTED;
  }
  for (int i = bitmask_length; i > 0; i--) {
    bitmask = (bitmask << 8) | frame_data[INDEX_BITMASK_LENGTH + i];
  }

  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(SUPPORTED_USER_TYPES),
                                     &bitmask,
                                     sizeof(bitmask));

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// User Credential Credential Capabilities Get/Report
/////////////////////////////////////////////////////////////////////////////
static sl_status_t
  zwave_command_class_user_credential_credential_capabilities_get(
    attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "Credential Capabilities Get");

  ZW_CREDENTIAL_CAPABILITIES_GET_FRAME *get_frame
    = (ZW_CREDENTIAL_CAPABILITIES_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_USER_CREDENTIAL;
  get_frame->cmd      = CREDENTIAL_CAPABILITIES_GET;
  *frame_length       = sizeof(ZW_CREDENTIAL_CAPABILITIES_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t
  zwave_command_class_user_credential_credential_capabilities_handle_report(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length < 5) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  sl_log_debug(LOG_TAG, "Credential Capabilities Report");

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  uint8_t support_credential_checksum
    = (frame_data[2]
       & CREDENTIAL_CAPABILITIES_REPORT_PROPERTIES1_CREDENTIAL_CHECKSUM_SUPPORT_BIT_MASK)
      > 0;

  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(SUPPORT_CREDENTIAL_CHECKSUM),
                                     &support_credential_checksum,
                                     sizeof(support_credential_checksum));

  uint8_t supported_credential_types_count = frame_data[3];

  // Remove all previous known CREDENTIAL_TYPE
  attribute_store_node_t type_node;
  do {
    type_node = attribute_store_get_node_child_by_type(
      endpoint_node,
      ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE),
      0);
    attribute_store_delete_node(type_node);
  } while (type_node != ATTRIBUTE_STORE_INVALID_NODE);

  uint16_t current_index = 4;
  for (uint8_t i = 0; i < supported_credential_types_count; i++) {
    // > Root node : Credential Type
    user_credential_type_t credential_type = frame_data[current_index];

    attribute_store_node_t credential_type_node
      = attribute_store_emplace(endpoint_node,
                                ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE),
                                &credential_type,
                                sizeof(credential_type));

    if (credential_type_node == ATTRIBUTE_STORE_INVALID_NODE) {
      sl_log_error(LOG_TAG, "Unable to create credential type node");
      return SL_STATUS_NOT_SUPPORTED;
    }

    sl_log_debug(LOG_TAG, "Supported credential type : %d", credential_type);

    // >> CL Support
    // Use same define as Credential Checksum Support here since it's a same and no define is available for cl support
    uint8_t cl_support_index = current_index + supported_credential_types_count;
    uint8_t support_cl
      = (frame_data[cl_support_index]
         & CREDENTIAL_CAPABILITIES_REPORT_PROPERTIES1_CREDENTIAL_CHECKSUM_SUPPORT_BIT_MASK)
        > 0;

    attribute_store_set_child_reported(
      credential_type_node,
      ATTRIBUTE(CREDENTIAL_LEARN_READ_BACK_SUPPORT),
      &support_cl,
      sizeof(support_cl));

    // >> 16 bits values
    auto store_uint16_value = [&](attribute_store_type_t type, uint8_t offset) {
      // First 16bit value after CL Support is cl_support_index + supported_credential_types_count
      // Then we add the 16 bit value offset to it :
      //  - CREDENTIAL_SUPPORTED_SLOT_COUNT [0] : supported_credential_types_count x 2 x [0] (base)
      //  - CREDENTIAL_MIN_LENGTH [1] : supported_credential_types_count x 2 x [1] (one batch of 16 bit data before)
      //  - CREDENTIAL_MAX_LENGTH [2] : supported_credential_types_count x 2 x [2] (two batch of 16 bit data before)
      // Then we add i to it (0..n) to make sure we take the right pair of data to form the 16 bit unsigned integer
      const uint16_t index = cl_support_index + supported_credential_types_count
                             + (supported_credential_types_count * 2 * offset)
                             + i;
      uint16_t value = get_uint16_value(frame_data, index);
      attribute_store_set_child_reported(credential_type_node,
                                         type,
                                         &value,
                                         sizeof(value));
    };

    store_uint16_value(ATTRIBUTE(CREDENTIAL_SUPPORTED_SLOT_COUNT), 0);
    store_uint16_value(ATTRIBUTE(CREDENTIAL_MIN_LENGTH), 1);
    store_uint16_value(ATTRIBUTE(CREDENTIAL_MAX_LENGTH), 2);

    current_index++;
  }

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// All User Checksum Get/Report
/////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_user_credential_all_user_checksum_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  sl_log_debug(LOG_TAG, "All User Checksum Get");

  ZW_ALL_USERS_CHECKSUM_GET_FRAME *get_frame
    = (ZW_ALL_USERS_CHECKSUM_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_USER_CREDENTIAL;
  get_frame->cmd      = ALL_USERS_CHECKSUM_GET;
  *frame_length       = sizeof(ZW_ALL_USERS_CHECKSUM_GET_FRAME);

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_all_user_checksum_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length != 4) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  sl_log_debug(LOG_TAG, "All User Checksum Report");

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  user_credential_all_users_checksum_t all_users_checksum
    = get_uint16_value(frame_data, 2);

  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(ALL_USERS_CHECKSUM),
                                     &all_users_checksum,
                                     sizeof(all_users_checksum));

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Credential Set/Get/Report
/////////////////////////////////////////////////////////////////////////////

// Start credential interview process by starting with 0,0
void trigger_get_credential(attribute_store_node_t user_unique_id_node,
                            user_credential_type_t credential_type,
                            user_credential_slot_t credential_slot)
{
  sl_log_debug(LOG_TAG,
               "Trigger GET credential for user %d : "
               "Credential type %d, credential slot %d",
               static_cast<uint16_t>(
                 attribute_store_get_reported_number(user_unique_id_node)),
               credential_type,
               credential_slot);

  // Create credential type node if it doesn't exists
  // Since the GET is mapped to the Credential SLOT we doesn't need to do anything specific here
  attribute_store_node_t credential_type_node
    = attribute_store_emplace(user_unique_id_node,
                              ATTRIBUTE(CREDENTIAL_TYPE),
                              &credential_type,
                              sizeof(credential_type));

  // Then check if credential slot node exists
  attribute_store_node_t credential_slot_node
    = attribute_store_get_node_child_by_value(credential_type_node,
                                              ATTRIBUTE(CREDENTIAL_SLOT),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&credential_slot,
                                              sizeof(credential_slot),
                                              0);

  // If it exists we clear it reported value and set it as desired
  if (attribute_store_node_exists(credential_slot_node)) {
    attribute_store_set_desired(credential_slot_node,
                                &credential_slot,
                                sizeof(credential_slot));
    attribute_store_undefine_reported(credential_slot_node);
  } else {  // If non existant we create it
    attribute_store_emplace_desired(credential_type_node,
                                    ATTRIBUTE(CREDENTIAL_SLOT),
                                    &credential_slot,
                                    sizeof(credential_slot));
  }
}

static sl_status_t zwave_command_class_user_credential_credential_set(
  attribute_store_node_t credential_operation_type_node,
  uint8_t *frame,
  uint16_t *frame_length)
{
  // Identifiers nodes
  attribute_store_node_t credential_slot_node
    = attribute_store_get_first_parent_with_type(credential_operation_type_node,
                                                 ATTRIBUTE(CREDENTIAL_SLOT));
  attribute_store_node_t credential_type_node
    = attribute_store_get_first_parent_with_type(credential_slot_node,
                                                 ATTRIBUTE(CREDENTIAL_TYPE));
  attribute_store_node_t user_unique_id_node
    = attribute_store_get_first_parent_with_type(credential_type_node,
                                                 ATTRIBUTE(USER_UNIQUE_ID));
  // Since CREDENTIAL_DATA is not directly under credential_slot_node we need to fetch it first
  attribute_store_node_t credential_length_node
    = attribute_store_get_first_child_by_type(
      credential_slot_node,
      ATTRIBUTE(CREDENTIAL_DATA_LENGTH));
  attribute_store_node_t credential_node
    = attribute_store_get_first_child_by_type(credential_length_node,
                                              ATTRIBUTE(CREDENTIAL_DATA));
  // Get operation type
  user_credential_operation_type_t operation_type = 0;
  sl_status_t status
    = attribute_store_get_desired(credential_operation_type_node,
                                  &operation_type,
                                  sizeof(operation_type));

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Can't get operation type. Not sending CREDENTIAL_SET.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  sl_log_debug(LOG_TAG,
               "Credential SET for Credential Slot %d, Credential Type %d, "
               "User %d (operation type : %d)",
               static_cast<user_credential_slot_t>(
                 attribute_store_get_reported_number(credential_slot_node)),
               static_cast<user_credential_type_t>(
                 attribute_store_get_reported_number(credential_type_node)),
               static_cast<user_credential_user_unique_id_t>(
                 attribute_store_get_reported_number(user_unique_id_node)),
               operation_type);

  // Since the data is not linear we provide the node directly
  std::vector<attribute_command_data> set_data
    = {{ATTRIBUTE(USER_UNIQUE_ID),
        DESIRED_OR_REPORTED_ATTRIBUTE,
        user_unique_id_node},
       {ATTRIBUTE(CREDENTIAL_TYPE),
        DESIRED_OR_REPORTED_ATTRIBUTE,
        credential_type_node},
       {ATTRIBUTE(CREDENTIAL_SLOT),
        DESIRED_OR_REPORTED_ATTRIBUTE,
        credential_slot_node},
       {ATTRIBUTE(CREDENTIAL_OPERATION_TYPE),
        DESIRED_ATTRIBUTE,
        credential_operation_type_node}};

  // Add the credential data if we are not trying to remove a credential
  if (operation_type != USER_CREDENTIAL_OPERATION_TYPE_DELETE) {
    set_data.push_back({ATTRIBUTE(CREDENTIAL_DATA),
                        DESIRED_OR_REPORTED_ATTRIBUTE,
                        credential_node});
  }
  status = create_command_frame(CREDENTIAL_SET,
                                set_data,
                                credential_slot_node,
                                frame,
                                frame_length);

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Can't create Credential SET frame");
    return SL_STATUS_NOT_SUPPORTED;
  }
  // If we are deleting the credential we are setting 0x00 as a credential length
  if (operation_type == USER_CREDENTIAL_OPERATION_TYPE_DELETE) {
    frame[*frame_length] = 0x00;
    *frame_length += 1;
  }

  return SL_STATUS_OK;
}

/**
 * @brief Credential GET
 * 
 * Attribute store IN : 
 *  CREDENTIAL_TYPE : desired value  => get_frame->credentialType
 *  CREDENTIAL_SLOT : desired value  => get_frame->credentialSlot1 & credentialSlot2
 *  USER_UNIQUE_ID  : reported value => get_frame->userUniqueIdentifier1 & userUniqueIdentifier2
 * 
 * Attribute store OUT :
 *  CREDENTIAL_TYPE : set reported as desired & clear desired
 *  CREDENTIAL_SLOT : set reported as desired & clear desired
 * 
 * @param credential_slot_node
 * @param frame 
 * @param frame_length 
 * @return sl_status_t 
 */
static sl_status_t zwave_command_class_user_credential_credential_get(
  attribute_store_node_t credential_slot_node,
  uint8_t *frame,
  uint16_t *frame_length)
{
  user_credential_slot_t credential_slot = 0;
  sl_status_t status = attribute_store_get_desired(credential_slot_node,
                                                   &credential_slot,
                                                   sizeof(credential_slot));
  if (status != SL_STATUS_OK) {
    sl_log_error(
      LOG_TAG,
      "Can't get credential slot value. Not sending CREDENTIAL_GET.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Get Credential type
  user_credential_type_t credential_type = 0;
  attribute_store_node_t credential_type_node
    = attribute_store_get_first_parent_with_type(credential_slot_node,
                                                 ATTRIBUTE(CREDENTIAL_TYPE));
  status = attribute_store_get_reported(credential_type_node,
                                        &credential_type,
                                        sizeof(credential_type));
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Can't get credential_type. Not sending CREDENTIAL_GET.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Get User Node
  attribute_store_node_t user_unique_id_node
    = attribute_store_get_first_parent_with_type(credential_type_node,
                                                 ATTRIBUTE(USER_UNIQUE_ID));
  user_credential_user_unique_id_t user_unique_id;
  status = attribute_store_get_reported(user_unique_id_node,
                                        &user_unique_id,
                                        sizeof(user_unique_id));
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Can't get user_unique_id. Not sending CREDENTIAL_GET.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Log credential node, type, and user ID
  sl_log_debug(
    LOG_TAG,
    "Credential Get. Credential slot: %d / Credential Type: %d (User %d)",
    credential_slot,
    credential_type,
    user_unique_id);

  auto exploded_credential_slot = explode_uint16(credential_slot);
  auto exploded_user_id         = explode_uint16(user_unique_id);

  ZW_CREDENTIAL_GET_FRAME *get_frame = (ZW_CREDENTIAL_GET_FRAME *)frame;
  get_frame->cmdClass                = COMMAND_CLASS_USER_CREDENTIAL;
  get_frame->cmd                     = CREDENTIAL_GET;
  get_frame->userUniqueIdentifier1   = exploded_user_id.msb;
  get_frame->userUniqueIdentifier2   = exploded_user_id.lsb;
  get_frame->credentialType          = credential_type;
  get_frame->credentialSlot1         = exploded_credential_slot.msb;
  get_frame->credentialSlot2         = exploded_credential_slot.lsb;

  *frame_length = sizeof(ZW_CREDENTIAL_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_credential_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Since INDEX_CREDENTIAL_DATA is a variable length field we know that we at lest expect 16 elements
  if (frame_length < 15) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  constexpr uint8_t INDEX_USER_ID              = 2;
  constexpr uint8_t INDEX_CREDENTIAL_TYPE      = 4;
  constexpr uint8_t INDEX_CREDENTIAL_SLOT      = 5;
  constexpr uint8_t INDEX_CREDENTIAL_READ_BACK = 7;
  constexpr uint8_t INDEX_CREDENTIAL_LENGTH    = 8;
  constexpr uint8_t INDEX_CREDENTIAL_DATA      = 9;
  const uint8_t credential_length = frame_data[INDEX_CREDENTIAL_LENGTH];
  const uint8_t INDEX_CREDENTIAL_MODIFIER_TYPE
    = INDEX_CREDENTIAL_DATA + credential_length;
  const uint8_t INDEX_CREDENTIAL_MODIFIER_NODE_ID
    = INDEX_CREDENTIAL_MODIFIER_TYPE + 1;
  const uint8_t INDEX_NEXT_CREDENTIAL_TYPE
    = INDEX_CREDENTIAL_MODIFIER_NODE_ID + 2;
  const uint8_t INDEX_NEXT_CREDENTIAL_SLOT = INDEX_NEXT_CREDENTIAL_TYPE + 1;

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  const user_credential_user_unique_id_t user_id
    = get_uint16_value(frame_data, INDEX_USER_ID);
  const user_credential_type_t credential_type
    = frame_data[INDEX_CREDENTIAL_TYPE];
  const user_credential_slot_t credential_slot
    = get_uint16_value(frame_data, INDEX_CREDENTIAL_SLOT);

  auto remove_credential_type_and_slot_0_if_exists = [&]() {
    attribute_store_node_t type_node_0;
    attribute_store_node_t slot_node_0;

    // Get nodes 0
    get_credential_type_node(endpoint_node, user_id, 0, REPORTED_ATTRIBUTE, type_node_0);
    get_credential_slot_node(type_node_0, 0, DESIRED_ATTRIBUTE, slot_node_0);

    // Remove them
    attribute_store_delete_node(type_node_0);
    attribute_store_delete_node(slot_node_0);
  };

  if (credential_type == 0 || credential_slot == 0) {
    sl_log_debug(LOG_TAG, "User %d has no credential to get", user_id);
    remove_credential_type_and_slot_0_if_exists();
    sl_log_debug(LOG_TAG, "Removed credential type and slot 0");
    return SL_STATUS_OK;
  }

  sl_log_debug(
    LOG_TAG,
    "Credential Report. Credential Type: %d / Credential Slot: %d (User %d)",
    credential_type,
    credential_slot,
    user_id);

  // We should have a valid user id if we receive this report
  attribute_store_node_t user_unique_id_node
    = get_reported_user_id_node(endpoint_node, user_id);

  // Check node existence
  if (!attribute_store_node_exists(user_unique_id_node)) {
    sl_log_error(LOG_TAG,
                 "Can't find user with ID %d in CREDENTIAL_REPORT",
                 user_id);
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Find credential type node based on user id and credential type
  attribute_store_node_t credential_type_node = ATTRIBUTE_STORE_INVALID_NODE;

  bool credential_type_found = get_credential_type_node(endpoint_node,
                                                        user_id,
                                                        credential_type,
                                                        REPORTED_ATTRIBUTE,
                                                        credential_type_node);

  // If not found we look for the credential type node of 0 (first interview)
  if (!credential_type_found) {
    // We don't care about the return value, if not defined it will be catch soon.
    get_credential_type_node(endpoint_node,
                             user_id,
                             0,
                             REPORTED_ATTRIBUTE,
                             credential_type_node);
  }

  // Now search for the slot node
  attribute_store_node_t credential_slot_node = ATTRIBUTE_STORE_INVALID_NODE;
  // Check reported value first
  if (!get_credential_slot_node(credential_type_node,
                                credential_slot,
                                REPORTED_ATTRIBUTE,
                                credential_slot_node)) {
    sl_log_debug(LOG_TAG,
                 "Could not find slot %d with reported value, trying desired",
                 credential_slot);
    // Then check desired value
    if (!get_credential_slot_node(credential_type_node,
                                  credential_slot,
                                  DESIRED_ATTRIBUTE,
                                  credential_slot_node)) {
      sl_log_debug(
        LOG_TAG,
        "Could not find slot %d with desired value, using the Slot with ID 0",
        credential_slot);

      // If not found it will be checked in the next step
      get_credential_slot_node(credential_type_node,
                               0,
                               DESIRED_ATTRIBUTE,
                               credential_slot_node);
    }
  }

  // Check if we could retrieve all the node we need
  if (!attribute_store_node_exists(credential_type_node)
      || !attribute_store_node_exists(credential_slot_node)) {
    sl_log_error(LOG_TAG,
                 "Can't find (Credential Type %d, Credential Slot %d) in "
                 "CREDENTIAL_REPORT",
                 credential_type,
                 credential_slot);
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Update credential slot node & type
  attribute_store_set_reported(credential_type_node,
                               &credential_type,
                               sizeof(credential_type));
  attribute_store_set_reported(credential_slot_node,
                               &credential_slot,
                               sizeof(credential_slot));
  // Since the get is listened on the credential slot node we need to clear it
  attribute_store_undefine_desired(credential_slot);

  // If there is any leftovers of slot 0 we remove them to prevent infinite loop
  remove_credential_type_and_slot_0_if_exists();

  // Set standard (uint8 & uint16) data
  std::vector<user_field_data> user_data = {
    {ATTRIBUTE(CREDENTIAL_READ_BACK),
     INDEX_CREDENTIAL_READ_BACK,
     CREDENTIAL_REPORT_PROPERTIES1_CRB_BIT_MASK,
     7},
    {ATTRIBUTE(CREDENTIAL_MODIFIER_TYPE), INDEX_CREDENTIAL_MODIFIER_TYPE},
    {ATTRIBUTE(CREDENTIAL_MODIFIER_NODE_ID), INDEX_CREDENTIAL_MODIFIER_NODE_ID},
  };

  sl_status_t status
    = set_reported_attributes(credential_slot_node, frame_data, user_data);

  if (status != SL_STATUS_OK) {
    return status;
  }

  // Set credential data
  auto credential_data_length_node
    = attribute_store_emplace(credential_slot_node,
                              ATTRIBUTE(CREDENTIAL_DATA_LENGTH),
                              &credential_length,
                              sizeof(credential_length));

  if (credential_data_length_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_error(LOG_TAG,
                 "Can't set CREDENTIAL_DATA_LENGTH in attribute store");
    return SL_STATUS_NOT_SUPPORTED;
  }

  status
    = attribute_store_set_child_reported(credential_data_length_node,
                                         ATTRIBUTE(CREDENTIAL_DATA),
                                         &frame_data[INDEX_CREDENTIAL_DATA],
                                         credential_length);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Can't set CREDENTIAL_DATA in attribute store");
    return SL_STATUS_NOT_SUPPORTED;
  }

  user_credential_type_t next_credential_type
    = frame_data[INDEX_NEXT_CREDENTIAL_TYPE];
  user_credential_slot_t next_credential_slot
    = get_uint16_value(frame_data, INDEX_NEXT_CREDENTIAL_SLOT);

  if (next_credential_type != 0 && next_credential_slot != 0) {
    trigger_get_credential(user_unique_id_node,
                           next_credential_type,
                           next_credential_slot);
    sl_log_debug(LOG_TAG,
                 "Next credential type and slot: %d, %d",
                 next_credential_type,
                 next_credential_slot);
  } else {
    sl_log_debug(LOG_TAG, "No more credential to get");
  }

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// User Set/Get/Report
/////////////////////////////////////////////////////////////////////////////

// Start user interview process by starting a user get with ID 0
void trigger_get_user(attribute_store_node_t endpoint_node,
                      user_credential_user_unique_id_t user_id)
{
  // If we are not in the special case of user ID 0 we need to check if user is already here
  if (user_id != 0) {
    attribute_store_node_t user_node
      = attribute_store_get_node_child_by_value(endpoint_node,
                                                ATTRIBUTE(USER_UNIQUE_ID),
                                                REPORTED_ATTRIBUTE,
                                                (uint8_t *)&user_id,
                                                sizeof(user_id),
                                                0);
    // If it exists we interview it again
    if (attribute_store_node_exists(user_node)) {
      sl_log_debug(
        LOG_TAG,
        "User Unique ID %d found. Undefine its reported value to update it.",
        user_id);
      attribute_store_set_desired(user_node, &user_id, sizeof(user_id));
      attribute_store_undefine_reported(user_node);
      return;
    }
  }

  // If user id is 0 or not existant we create it
  sl_log_debug(LOG_TAG, "Creating User Unique ID node %d", user_id);
  attribute_store_emplace_desired(endpoint_node,
                                  ATTRIBUTE(USER_UNIQUE_ID),
                                  &user_id,
                                  sizeof(user_id));
}

static sl_status_t zwave_command_class_user_credential_user_set(
  attribute_store_node_t user_operation_type_node,
  uint8_t *frame,
  uint16_t *frame_length)
{
  // Get user unique id node
  attribute_store_node_t user_unique_id_node
    = attribute_store_get_first_parent_with_type(user_operation_type_node,
                                                 ATTRIBUTE(USER_UNIQUE_ID));

  user_credential_user_unique_id_t user_id = 0;
  attribute_store_get_desired_else_reported(user_unique_id_node,
                                            &user_id,
                                            sizeof(user_id));

  sl_log_debug(LOG_TAG, "User SET for user %d", user_id);

  // If we can't get the user unique id we can't continue
  if (!attribute_store_node_exists(user_unique_id_node)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  user_credential_operation_type_t user_operation_type = 0;
  sl_status_t status = attribute_store_get_desired(user_operation_type_node,
                              &user_operation_type,
                              sizeof(user_operation_type));

  if (status != SL_STATUS_OK) { 
    sl_log_error(LOG_TAG,
                 "Can't get user operation type value. Not sending USER_SET.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  std::vector<attribute_command_data> set_data = {
      {ATTRIBUTE(USER_OPERATION_TYPE), DESIRED_ATTRIBUTE},
      {ATTRIBUTE(USER_UNIQUE_ID), DESIRED_OR_REPORTED_ATTRIBUTE, user_unique_id_node}
  };

  // If we are not deleting the user we need more data
  if (user_operation_type != USER_SET_OPERATION_TYPE_DELETE) {
    std::vector<attribute_command_data> extra_set_data = {
      {ATTRIBUTE(USER_TYPE), DESIRED_OR_REPORTED_ATTRIBUTE},
      {ATTRIBUTE(USER_ACTIVE_STATE), DESIRED_OR_REPORTED_ATTRIBUTE},
      {ATTRIBUTE(CREDENTIAL_RULE), DESIRED_OR_REPORTED_ATTRIBUTE},
      {ATTRIBUTE(USER_EXPIRING_TIMEOUT_MINUTES), DESIRED_OR_REPORTED_ATTRIBUTE},
      {ATTRIBUTE(USER_NAME_ENCODING), DESIRED_OR_REPORTED_ATTRIBUTE},
      {ATTRIBUTE(USER_NAME), DESIRED_OR_REPORTED_ATTRIBUTE},
    };
    std::copy(extra_set_data.begin(),
              extra_set_data.end(),
              std::back_inserter(set_data));
  }

  status = create_command_frame(USER_SET,
                                set_data,
                                user_unique_id_node,
                                frame,
                                frame_length);

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Can't create User SET frame");
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_user_credential_user_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  user_credential_user_unique_id_t user_id = 0;
  sl_status_t status
    = attribute_store_get_desired(node, &user_id, sizeof(user_id));

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Can't get user unique id value. Not sending USER_GET.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  sl_log_debug(LOG_TAG, "User Get for user %d", user_id);

  auto exploded_value = explode_uint16(user_id);

  ZW_USER_GET_FRAME *get_frame     = (ZW_USER_GET_FRAME *)frame;
  get_frame->cmdClass              = COMMAND_CLASS_USER_CREDENTIAL;
  get_frame->cmd                   = USER_GET;
  get_frame->userUniqueIdentifier1 = exploded_value.msb;
  get_frame->userUniqueIdentifier2 = exploded_value.lsb;

  *frame_length = sizeof(ZW_USER_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_user_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  constexpr uint8_t INDEX_NEXT_USER_ID                       = 2;
  constexpr uint8_t INDEX_USER_MODIFIER_TYPE                 = 4;
  constexpr uint8_t INDEX_USER_MODIFIER_ID                   = 5;
  constexpr uint8_t INDEX_USER_ID                            = 7;
  constexpr uint8_t INDEX_USER_TYPE                          = 9;
  constexpr uint8_t INDEX_USER_ACTIVE_STATE                  = 10;
  constexpr uint8_t INDEX_CREDENTIAL_RULE                    = 11;
  constexpr uint8_t INDEX_USER_NAME_EXPIRING_TIMEOUT_MINUTES = 12;
  constexpr uint8_t INDEX_USER_NAME_ENCODING                 = 14;
  constexpr uint8_t INDEX_USER_NAME_LENGTH                   = 15;
  constexpr uint8_t INDEX_USER_NAME                          = 16;

  if (frame_length < INDEX_USER_NAME) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  // Find the user
  const user_credential_user_unique_id_t user_id
    = get_uint16_value(frame_data, INDEX_USER_ID);

  sl_log_debug(LOG_TAG, "User report for user %d", user_id);

  auto remove_node_0_if_exists = [&]() {
    attribute_store_node_t node_0;
    get_user_id_node(endpoint_node, 0, DESIRED_ATTRIBUTE, node_0);
    return attribute_store_delete_node(node_0);
  };

  // CC:0083.01.05.11.006: Zero is an invalid User Unique Identifier and MUST NOT be used by the node
  if (user_id == 0) {
    sl_log_info(LOG_TAG,
                "User report with ID 0 received. This probably means that no "
                "user is defined on the device.");
    sl_log_debug(LOG_TAG,
                 "Attempt to delete User Node ID with value %d",
                 user_id);
    sl_status_t deletion_status = remove_node_0_if_exists();
    sl_log_debug(LOG_TAG, "Deletion returned status : %d", deletion_status);
    return SL_STATUS_OK;
  }

  // Get User ID node
  attribute_store_node_t user_unique_id_node;

  // First we check if there this user already exists in our database (reported value)
  if (!get_user_id_node(endpoint_node,
                        user_id,
                        REPORTED_ATTRIBUTE,
                        user_unique_id_node)) {
    sl_log_debug(LOG_TAG,
                 "Could not find user %d with reported value, trying desired",
                 user_id);
    // If User node doesn't exists with given desired attribute
    if (!get_user_id_node(endpoint_node,
                          user_id,
                          DESIRED_ATTRIBUTE,
                          user_unique_id_node)) {
      sl_log_debug(
        LOG_TAG,
        "Could not find user %d with desired value, using the User ID 0",
        user_id);

      // If not found it will be checked in the next step
      get_user_id_node(endpoint_node,
                       0,
                       DESIRED_ATTRIBUTE,
                       user_unique_id_node);
    }
  }

  // Check node existence
  if (!attribute_store_node_exists(user_unique_id_node)) {
    sl_log_error(LOG_TAG, "Can't find user with ID %d in USER_REPORT", user_id);
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Check if user still exists
  if (frame_data[INDEX_USER_MODIFIER_TYPE] == USER_REPORT_DNE) {
    sl_log_debug(LOG_TAG, "User %d does not exist anymore, removing from attribute store.", user_id);
    attribute_store_delete_node(user_unique_id_node);
    return SL_STATUS_OK;
  }

  // Set reported value
  attribute_store_set_reported(user_unique_id_node, &user_id, sizeof(user_id));
  attribute_store_undefine_desired(user_unique_id_node);

  // Remove leftover of node 0 if it exists
  // This is necessary if we are interviewing again the user as this node will be left undefined
  // and causing a get loop.
  remove_node_0_if_exists();

  // Set standard (uint8 & uint16) data
  std::vector<user_field_data> user_data
    = {{ATTRIBUTE(USER_MODIFIER_TYPE), INDEX_USER_MODIFIER_TYPE},
       {ATTRIBUTE(USER_MODIFIER_NODE_ID), INDEX_USER_MODIFIER_ID},
       {ATTRIBUTE(USER_TYPE), INDEX_USER_TYPE},
       {ATTRIBUTE(USER_ACTIVE_STATE),
        INDEX_USER_ACTIVE_STATE,
        USER_REPORT_PROPERTIES1_USER_ACTIVE_STATE_BIT_MASK},
       {ATTRIBUTE(USER_NAME_ENCODING),
        INDEX_USER_NAME_ENCODING,
        USER_REPORT_PROPERTIES2_USER_NAME_ENCODING_MASK},
       {ATTRIBUTE(CREDENTIAL_RULE), INDEX_CREDENTIAL_RULE},
       {ATTRIBUTE(USER_EXPIRING_TIMEOUT_MINUTES),
        INDEX_USER_NAME_EXPIRING_TIMEOUT_MINUTES}};

  sl_status_t set_status
    = set_reported_attributes(user_unique_id_node, frame_data, user_data);

  if (set_status != SL_STATUS_OK) {
    return set_status;
  }

  // User name
  uint8_t user_name_length = frame_data[INDEX_USER_NAME_LENGTH];
  std::string user_name
    = get_string_value(frame_data, INDEX_USER_NAME, user_name_length);

  // Prevent duplicate
  auto user_name_node
    = attribute_store_get_first_child_by_type(user_unique_id_node,
                                              ATTRIBUTE(USER_NAME));
  if (!attribute_store_node_exists(user_name_node)) {
    user_name_node
      = attribute_store_add_node(ATTRIBUTE(USER_NAME), user_unique_id_node);
  }
  attribute_store_set_reported_string(user_name_node, user_name.c_str());

  // Get credentials
  trigger_get_credential(user_unique_id_node, 0, 0);

  // Interview next ID if needed
  user_credential_user_unique_id_t next_user_id
    = get_uint16_value(frame_data, INDEX_NEXT_USER_ID);
  if (next_user_id != 0) {
    sl_log_debug(LOG_TAG, "Trigger a get for next user (%d)", next_user_id);
    trigger_get_user(endpoint_node, next_user_id);
  } else {
    sl_log_debug(LOG_TAG, "No more users to discover");
  }

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Post interview actions
/////////////////////////////////////////////////////////////////////////////
void zwave_network_status_changed(attribute_store_node_t updated_node,
                                  attribute_store_change_t change)
{
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_NODE_ID);

  zwave_node_id_t node_id;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));

  // If we are updating the zpc node or if we trying to delete the attribute we don't want to do anything
  if (change == ATTRIBUTE_DELETED || get_zpc_node_id_node() == node_id_node) {
    return;
  }

  NodeStateNetworkStatus network_status;
  sl_status_t reported_value_status
    = attribute_store_get_reported(updated_node,
                                   &network_status,
                                   sizeof(network_status));

  // If the endpoint report is marked as ONLINE_FUNCTIONAL
  if (reported_value_status == SL_STATUS_OK
      && network_status == ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL) {
    sl_log_debug(LOG_TAG,
                 "Node %d is now ONLINE_FUNCTIONAL : start the delayed "
                 "interview process",
                 node_id);
    // Perform action on each endpoint that supports User Credential Command class
    uint8_t endpoint_count
      = attribute_store_get_node_child_count_by_type(node_id_node,
                                                     ATTRIBUTE_ENDPOINT_ID);

    sl_log_debug(LOG_TAG, "Checking endpoints (total : %d)...", endpoint_count);

    for (uint8_t i = 0; i < endpoint_count; i++) {
      // Get current endpoint node
      attribute_store_node_t endpoint_node
        = attribute_store_get_node_child_by_type(node_id_node,
                                                 ATTRIBUTE_ENDPOINT_ID,
                                                 i);

      zwave_endpoint_id_t endpoint_id;
      attribute_store_get_reported(endpoint_node,
                                   &endpoint_id,
                                   sizeof(endpoint_id));
      // Check if the endpoint supports User Credential Command class
      if (zwave_node_supports_command_class(COMMAND_CLASS_USER_CREDENTIAL,
                                            node_id,
                                            endpoint_id)) {
        auto user_count = attribute_store_get_node_child_count_by_type(
          endpoint_node,
          ATTRIBUTE(USER_UNIQUE_ID));
        sl_log_debug(LOG_TAG,
                     "Endpoint %d supports User Credential. Removing all "
                     "existing user nodes (%d)...",
                     i,
                     user_count);
        for (size_t j = 0; j < user_count; j++) {
          // Delete the first attribute we find until we have no more left
          attribute_store_node_t user_node
            = attribute_store_get_node_child_by_type(endpoint_node,
                                                     ATTRIBUTE(USER_UNIQUE_ID),
                                                     0);
          attribute_store_delete_node(user_node);
        }

        sl_log_debug(LOG_TAG, "Starting User and Credential interview");
        // Start the interview process with user ID = 0
        trigger_get_user(endpoint_node, 0);
      }
    }
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