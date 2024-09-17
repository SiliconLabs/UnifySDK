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
#include "user_credential_cluster_server.h"
#include "zcl_cluster_servers_helpers.hpp"
// Interfaces
#include "zwave_command_class_user_credential.h"

// ZPC includes
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_command_class_generic_types.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_store_type_registration.h"

// Includes from Unify shared components
#include "attribute.hpp"
#include "attribute_store_helper.h"
#include "sl_log.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_helpers.hpp"
#include "zap-types.h"

// Generic includes
#include <string>
#include <vector>
#include <stdlib.h>
#include <map>

// JSON
#include <nlohmann/json.hpp>

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

// UTF16 conversion (deprecated in C++17)
// Needed for credential data (password) per specification
#include <locale>
#include <codecvt>

// Setup Log ID
constexpr char LOG_TAG[] = "user_credential_cluster_server";

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_##type

///////////////////////////////////////////////////////////////////////
// Data structures
///////////////////////////////////////////////////////////////////////
// Data structure to hold the MQTT topic and conversion function (if needed) for each attribute
struct mqtt_data {
  std::string topic;
  std::function<std::string(uint32_t)> convert_function = nullptr;
};
// Used to bind an attribute to a MQTT topic
typedef std::map<attribute_store_type_t, mqtt_data> user_attributes_mqtt_map_t;

///////////////////////////////////////////////////////////////////////
// Conversion functions
///////////////////////////////////////////////////////////////////////
std::string convert_to_bool(uint32_t value)
{
  return value ? "true" : "false";
}

///////////////////////////////////////////////////////////////////////
// Static variables
///////////////////////////////////////////////////////////////////////
// User attributes
static const user_attributes_mqtt_map_t user_attributes = {
  {ATTRIBUTE(USER_TYPE), {"UserType", user_type_enum_get_enum_value_name}},
  {ATTRIBUTE(USER_ACTIVE_STATE), {"UserActiveState", convert_to_bool}},
  {ATTRIBUTE(CREDENTIAL_RULE),
   {"CredentialRule", cred_rule_get_enum_value_name}},
  {ATTRIBUTE(USER_EXPIRING_TIMEOUT_MINUTES), {"ExpiringTimeoutMinutes"}},
  {ATTRIBUTE(USER_NAME_ENCODING),
   {"UserNameEncoding", user_name_encoding_type_get_enum_value_name}},
  {ATTRIBUTE(USER_MODIFIER_TYPE),
   {"UserModifierType", user_modifier_type_get_enum_value_name}},
  {ATTRIBUTE(USER_MODIFIER_NODE_ID), {"UserModifierNodeId"}},
  {ATTRIBUTE(USER_NAME), {"UserName"}},
};

// Credential attributes
static const user_attributes_mqtt_map_t credential_attributes
  = {{ATTRIBUTE(CREDENTIAL_READ_BACK), {"CredentialReadBack", convert_to_bool}},
     {ATTRIBUTE(CREDENTIAL_MODIFIER_TYPE),
      {"CredentialModifierType", user_modifier_type_get_enum_value_name}},
     {ATTRIBUTE(CREDENTIAL_MODIFIER_NODE_ID), {"CredentialModifierNodeId"}},
     {ATTRIBUTE(CREDENTIAL_DATA), {"CredentialData"}}};

static const user_attributes_mqtt_map_t credential_rules_attributes
  = {{ATTRIBUTE(CREDENTIAL_LEARN_SUPPORT), {"LearnSupport", convert_to_bool}},
     {ATTRIBUTE(CREDENTIAL_SUPPORTED_SLOT_COUNT), {"SupportedSlotCount"}},
     {ATTRIBUTE(CREDENTIAL_MIN_LENGTH), {"CredentialMinLength"}},
     {ATTRIBUTE(CREDENTIAL_MAX_LENGTH), {"CredentialMaxLength"}},
     {ATTRIBUTE(CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT),
      {"LearnRecommendedTimeout"}},
     {ATTRIBUTE(CREDENTIAL_LEARN_NUMBER_OF_STEPS), {"LearnNumberOfSteps"}}};

///////////////////////////////////////////////////////////////////////////////
// DotDot MQTT incoming commands handling functions
//////////////////////////////////////////////////////////////////////////////
static sl_status_t
  add_user_command(dotdot_unid_t unid,
                   dotdot_endpoint_id_t endpoint,
                   uic_mqtt_dotdot_callback_call_type_t call_type,
                   uint16_t user_uniqueid,
                   UserTypeEnum user_type,
                   bool user_active_state,
                   CredRule credential_rule,
                   const char *user_name,
                   uint16_t expiring_timeout_minutes,
                   UserNameEncodingType user_name_encoding)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    attribute_store_node_t user_count_node
      = attribute_store_get_first_child_by_type(endpoint_node,
                                                ATTRIBUTE(NUMBER_OF_USERS));

    return attribute_store_node_exists(user_count_node) ? SL_STATUS_OK
                                                        : SL_STATUS_FAIL;
  }

  return zwave_command_class_user_credential_add_new_user(
    endpoint_node,
    user_uniqueid,
    static_cast<user_credential_user_type_t>(user_type),
    static_cast<user_credential_rule_t>(credential_rule),
    user_active_state,
    expiring_timeout_minutes,
    static_cast<user_credential_user_name_encoding_t>(user_name_encoding),
    user_name);
}

static sl_status_t
  modify_user_command(dotdot_unid_t unid,
                      dotdot_endpoint_id_t endpoint,
                      uic_mqtt_dotdot_callback_call_type_t call_type,
                      uint16_t user_uniqueid,
                      UserTypeEnum user_type,
                      bool user_active_state,
                      CredRule credential_rule,
                      const char *user_name,
                      uint16_t expiring_timeout_minutes,
                      UserNameEncodingType user_name_encoding)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    attribute_store_node_t user_count_node
      = attribute_store_get_first_child_by_type(endpoint_node,
                                                ATTRIBUTE(NUMBER_OF_USERS));

    return attribute_store_node_exists(user_count_node) ? SL_STATUS_OK
                                                        : SL_STATUS_FAIL;
  }

  return zwave_command_class_user_credential_modify_user(
    endpoint_node,
    user_uniqueid,
    static_cast<user_credential_user_type_t>(user_type),
    static_cast<user_credential_rule_t>(credential_rule),
    user_active_state,
    expiring_timeout_minutes,
    static_cast<user_credential_user_name_encoding_t>(user_name_encoding),
    user_name);

  return SL_STATUS_OK;
}

static sl_status_t
  delete_user_command(dotdot_unid_t unid,
                      dotdot_endpoint_id_t endpoint,
                      uic_mqtt_dotdot_callback_call_type_t call_type,
                      uint16_t user_uniqueid)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    attribute_store_node_t user_count_node
      = attribute_store_get_first_child_by_type(endpoint_node,
                                                ATTRIBUTE(NUMBER_OF_USERS));

    return attribute_store_node_exists(user_count_node) ? SL_STATUS_OK
                                                        : SL_STATUS_FAIL;
  }

  return zwave_command_class_user_credential_delete_user(endpoint_node,
                                                         user_uniqueid);
}

static sl_status_t
  add_credential_command(dotdot_unid_t unid,
                         dotdot_endpoint_id_t endpoint,
                         uic_mqtt_dotdot_callback_call_type_t call_type,
                         uint16_t user_uniqueid,
                         CredType credential_type,
                         uint16_t credential_slot,
                         const char *credential_data)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    attribute_store_node_t user_count_node
      = attribute_store_get_first_child_by_type(endpoint_node,
                                                ATTRIBUTE(NUMBER_OF_USERS));

    return attribute_store_node_exists(user_count_node) ? SL_STATUS_OK
                                                        : SL_STATUS_FAIL;
  }

  return zwave_command_class_user_credential_add_new_credential(
    endpoint_node,
    user_uniqueid,
    static_cast<user_credential_type_t>(credential_type),
    credential_slot,
    credential_data);
}

static sl_status_t
  modify_credential_command(dotdot_unid_t unid,
                            dotdot_endpoint_id_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type,
                            uint16_t user_uniqueid,
                            CredType credential_type,
                            uint16_t credential_slot,
                            const char *credential_data)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    attribute_store_node_t user_count_node
      = attribute_store_get_first_child_by_type(endpoint_node,
                                                ATTRIBUTE(NUMBER_OF_USERS));
    return attribute_store_node_exists(user_count_node) ? SL_STATUS_OK
                                                        : SL_STATUS_FAIL;
  }
  return zwave_command_class_user_credential_modify_credential(
    endpoint_node,
    user_uniqueid,
    static_cast<user_credential_type_t>(credential_type),
    credential_slot,
    credential_data);
}

static sl_status_t
  delete_credential_command(dotdot_unid_t unid,
                            dotdot_endpoint_id_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type,
                            uint16_t user_uniqueid,
                            CredType credential_type,
                            uint16_t credential_slot)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    attribute_store_node_t user_count_node
      = attribute_store_get_first_child_by_type(endpoint_node,
                                                ATTRIBUTE(NUMBER_OF_USERS));

    return attribute_store_node_exists(user_count_node) ? SL_STATUS_OK
                                                        : SL_STATUS_FAIL;
  }

  return zwave_command_class_user_credential_delete_credential(
    endpoint_node,
    user_uniqueid,
    static_cast<user_credential_type_t>(credential_type),
    credential_slot);
}

///////////////////////////////////////////////////////////////////////////////
// Helpers functions
//////////////////////////////////////////////////////////////////////////////
void register_attributes_to_mqtt_map(
  const user_attributes_mqtt_map_t &map,
  attribute_store_node_changed_callback_t callback)
{
  sl_status_t status = SL_STATUS_OK;
  for (const auto &[attribute_store_type, mqtt_data]: map) {
    status |= attribute_store_register_callback_by_type_and_state(
      callback,
      attribute_store_type,
      REPORTED_ATTRIBUTE);
  }
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Failed to register callbacks for User Credential");
  }
}

///////////////////////////////////////////////////////////////////////////////
// MQTT Helpers functions
//////////////////////////////////////////////////////////////////////////////

/**
 * @brief Get the base MQTT topic for a UserCredential.
 * 
 * This function will return the base MQTT topic for a UserCredential based on the
 * updated node. The topic will be in the format:
 * ucl/by-unid/<unid>/ep<endpoint>/UserCredential/Attributes
*/
std::string get_base_mqtt_topic(attribute_store::attribute updated_node_cpp)
{
  boost::format mqtt_topic
    = boost::format("ucl/by-unid/%1%/ep%2%/UserCredential/Attributes");

  // Go up and find the UNID/Endpoint and its network status.
  unid_t unid;
  zwave_endpoint_id_t endpoint_id = 0;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_unid_endpoint_from_node(
        updated_node_cpp,
        unid,
        &endpoint_id)) {
    return "";
  }

  // Do not publish any state supported commands for ourselves.
  if (is_zpc_unid(unid)) {
    return "";
  }

  // Need to convert endpoint_id to an unsigned int to be displayed as an int and not a char
  return (mqtt_topic % unid % static_cast<unsigned int>(endpoint_id)).str();
}

/**
 * @brief Get the base MQTT topic for a user.
 *
 * This function will return the base MQTT topic for a user based on the
 * updated node. The topic will be in the format:
 * ucl/by-unid/<unid>/ep<endpoint>/UserCredential/Attributes/User/<user_id>
 *
 * @param updated_node_cpp The node that will be published.
 * @return The base MQTT topic for the user.
 */
std::string
  get_base_user_mqtt_topic(attribute_store::attribute updated_node_cpp)
{
  boost::format mqtt_topic
    = boost::format("%1%/User/%2%");

  std::string base_mqtt_topic = get_base_mqtt_topic(updated_node_cpp);
  if (base_mqtt_topic.empty()) {
    return "";
  }

  try {
    user_credential_user_unique_id_t user_id
      = updated_node_cpp.first_parent_or_self(ATTRIBUTE(USER_UNIQUE_ID))
          .reported<user_credential_user_unique_id_t>();

    return (mqtt_topic % base_mqtt_topic  % user_id)
      .str();
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while publishing User attribute (%s) : %s",
                 attribute_store_type_get_node_type_name(updated_node_cpp),
                 e.what());
  }

  return "";
}

/**
 * @brief Get the base MQTT topic for a credential.
 *
 * This function will return the base MQTT topic for a credential based on the
 * updated node. The topic will be in the format:
 * ucl/by-unid/<unid>/ep<endpoint>/UserCredential/Attributes/User/<user_id>/Credential/<credential_node>/<credential_type>/<attribute>/Reported
 *
 * @param updated_node_cpp The updated node.
 * @return The base MQTT topic for the credential.
 */
std::string
  get_base_credential_mqtt_topic(attribute_store::attribute updated_node_cpp)
{
  std::string base_user_mqtt_topic = get_base_user_mqtt_topic(updated_node_cpp);
  if (base_user_mqtt_topic.empty()) {
    return "";
  }

  try {
    user_credential_slot_t credential_slot
      = updated_node_cpp.first_parent(ATTRIBUTE(CREDENTIAL_SLOT))
          .reported<user_credential_slot_t>();

    user_credential_type_t credential_type
      = updated_node_cpp.first_parent(ATTRIBUTE(CREDENTIAL_TYPE))
          .reported<user_credential_type_t>();
    std::string credential_type_str
      = cred_type_get_enum_value_name(credential_type);

    return (boost::format("%1%/Credential/%2%/%3%") % base_user_mqtt_topic
            % credential_type_str % credential_slot)
      .str();
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while publishing Credential attribute (%s) : %s",
                 attribute_store_type_get_node_type_name(updated_node_cpp),
                 e.what());
  }

  return "";
}

std::string
  get_base_credential_rule_mqtt_topic(attribute_store::attribute updated_node_cpp)
{
  std::string base_user_mqtt_topic = get_base_mqtt_topic(updated_node_cpp);
  if (base_user_mqtt_topic.empty()) {
    return "";
  }

  try {
    user_credential_type_t credential_type
      = updated_node_cpp.first_parent(ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE))
          .reported<user_credential_type_t>();
    std::string credential_type_str
      = cred_type_get_enum_value_name(credential_type);

    return (boost::format("%1%/Credentials/%2%") % base_user_mqtt_topic
            % credential_type_str)
      .str();
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while publishing Credential Rule attribute (%s) : %s",
                 attribute_store_type_get_node_type_name(updated_node_cpp),
                 e.what());
  }

  return "";
}

/**
 * @brief Add an attribute to the MQTT topic.
 *
 * This function will add an attribute to the MQTT topic. The topic will be in
 * the format:
 * <base_mqtt_topic>/<attribute_name>/Reported
 *
 * @param base_mqtt_topic The base MQTT topic.
 * @param attribute_name The attribute name to add.
 * @return The full MQTT topic.
 */
std::string mqtt_topic_add_attribute(const std::string &base_mqtt_topic,
                                     const std::string &attribute_name)
{
  return (boost::format("%1%/%2%/Reported") % base_mqtt_topic % attribute_name)
    .str();
}

/**
 * @brief Get the payload value for the attribute.
 *
 * This function will get the payload value for the attribute. The payload will
 * be in the format:
 * {
 *   "value": <value>
 * }
 *
 * @param updated_node_cpp The updated node.
 * @param data The MQTT data.
 * @param need_utf8_conversion If the value needs to be converted to UTF-8. Only applies to C_STRING_STORAGE_TYPE.
 * 
 * @return The payload value.
 */
std::string get_payload_value(attribute_store::attribute updated_node_cpp,
                              mqtt_data data,
                              bool need_utf8_conversion = false)
{
  std::string payload_str = "";

  auto node_storage_type
    = attribute_store_get_storage_type(updated_node_cpp.type());
  nlohmann::json json_payload;
  switch (node_storage_type) {
    case U8_STORAGE_TYPE:
      json_payload["value"] = updated_node_cpp.reported<uint8_t>();
      break;
    case U16_STORAGE_TYPE:
      json_payload["value"] = updated_node_cpp.reported<uint16_t>();
      break;
    case U32_STORAGE_TYPE:
      json_payload["value"] = updated_node_cpp.reported<uint32_t>();
      break;
    case C_STRING_STORAGE_TYPE:
      json_payload["value"] = updated_node_cpp.reported<std::string>();
      break;
    case BYTE_ARRAY_STORAGE_TYPE: {
      // Convert the byte array to a string
      // Credential data are encoded in different way that's why we store them as raw byte array and interpret their value
      
      std::string output;
      auto data = updated_node_cpp.reported<std::vector<uint8_t>>();

      // Check if we need to convert the value to UTF-8
      if (need_utf8_conversion) {
        std::u16string utf16_str;
        for(size_t i=0; i<data.size(); i+=2) {
          char16_t char16 = (data[i] << 8) | data[i+1];
          utf16_str.push_back(char16);
        }
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> cnv;
        output = cnv.to_bytes(utf16_str);
        if (cnv.converted() < utf16_str.size()) {
          sl_log_error(LOG_TAG, "Error converting UTF-16 to UTF-8");
        }
      } else {
        output.reserve(data.size());
        for (auto c: data) {
          output.push_back(static_cast<char>(c));
        }
      }

      json_payload["value"] = output;
    } break;

    default:
      sl_log_error(LOG_TAG,
                   "Unsupported storage type for user credential attribute");
      return "";
  }

  // Check if we need to convert the enum to a string
  const auto &enum_name_conversion = data.convert_function;
  if (enum_name_conversion) {
    json_payload["value"] = enum_name_conversion(json_payload["value"]);
  }

  // Create payload
  payload_str = json_payload.dump();
  // Handle booleans
  if (enum_name_conversion) {
    boost::replace_all(payload_str, "\"true\"", "true");
    boost::replace_all(payload_str, "\"false\"", "false");
  }

  return payload_str;
}

void publish_mqtt_topic(const std::string &base_topic,
                        mqtt_data data,
                        attribute_store::attribute updated_node_cpp,
                        attribute_store_change_t change)
{
  std::string mqtt_topic = mqtt_topic_add_attribute(base_topic, data.topic);
  sl_log_debug(LOG_TAG, "Publishing to topic: %s", mqtt_topic.c_str());

  std::string payload_str = "";
  if (change != ATTRIBUTE_DELETED) {

    if (updated_node_cpp.type() == ATTRIBUTE(CREDENTIAL_DATA)) {
      // Credential data are encoded in different way that's why we store them as raw byte array and interpret their value
      // We need to convert the value to UTF-8 if the credential type is password
      user_credential_type_t type
        = updated_node_cpp.first_parent(ATTRIBUTE(CREDENTIAL_TYPE))
            .reported<user_credential_type_t>();
      payload_str = get_payload_value(updated_node_cpp,
                                      data,
                                      (type == ZCL_CRED_TYPE_PASSWORD));
    } else {
      payload_str = get_payload_value(updated_node_cpp, data);
    }
    if (payload_str.empty()) {
      return;
    }
    sl_log_debug(LOG_TAG, "Payload %s", payload_str.c_str());
  } else {
    sl_log_debug(LOG_TAG, "Deletion of attribute, sending empty payload");
  }

  // Publish topic
  uic_mqtt_publish(mqtt_topic.c_str(),
                   payload_str.c_str(),
                   payload_str.length(),
                   true);
}

///////////////////////////////////////////////////////////////////////////////
// Attribute monitoring
//////////////////////////////////////////////////////////////////////////////

/**
 * @brief Callback for when an attribute is updated.
 * 
 * This function will be called when an attribute is updated. It will then
 * publish the updated attribute to the MQTT broker.
 * 
 * @param updated_node The updated node.
 * @param change The change that occurred.
 * @param get_base_mqtt_topic The function to get the base MQTT topic.
 * @param attributes The attributes data to publish.
*/
void on_attribute_update(
  attribute_store_node_t updated_node,
  attribute_store_change_t change,
  std::function<std::string(attribute_store::attribute)> get_base_mqtt_topic,
  const user_attributes_mqtt_map_t &attributes)
{
  try {
    auto updated_node_cpp = attribute_store::attribute(updated_node);
    // If no reported value exists, we wait for it
    if (!updated_node_cpp.reported_exists()) {
      return;
    }

    // Build the base topic and pass it on to DotDot MQTT.
    std::string mqtt_topic = get_base_mqtt_topic(updated_node_cpp);
    if (mqtt_topic.empty()) {
      return;
    }

    const auto mqtt_data = attributes.at(updated_node_cpp.type());

    publish_mqtt_topic(mqtt_topic, mqtt_data, updated_node_cpp, change);

  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while publishing User Credential attribute: %s",
                 e.what());
  }
}

void on_user_update(attribute_store_node_t updated_node,
                    attribute_store_change_t change)
{
  on_attribute_update(updated_node,
                      change,
                      &get_base_user_mqtt_topic,
                      user_attributes);
}

void on_credential_rules_update(attribute_store_node_t updated_node,
                                attribute_store_change_t change)
{
  on_attribute_update(updated_node,
                      change,
                      &get_base_credential_rule_mqtt_topic,
                      credential_rules_attributes);
}

void on_credential_update(attribute_store_node_t updated_node,
                          attribute_store_change_t change)
{
  on_attribute_update(updated_node,
                      change,
                      &get_base_credential_mqtt_topic,
                      credential_attributes);
}

///////////////////////////////////////////////////////////////////////////////
//  Init and teardown functions.
//////////////////////////////////////////////////////////////////////////////
sl_status_t user_credential_cluster_server_init()
{
  sl_log_debug(LOG_TAG, "User Credential server initialization");

  // User properties update
  register_attributes_to_mqtt_map(user_attributes, &on_user_update);
  register_attributes_to_mqtt_map(credential_attributes, &on_credential_update);
  register_attributes_to_mqtt_map(credential_rules_attributes,
                                  &on_credential_rules_update);

  // Command callbacks
  // User
  uic_mqtt_dotdot_user_credential_add_user_callback_set(&add_user_command);
  uic_mqtt_dotdot_user_credential_modify_user_callback_set(
    &modify_user_command);
  uic_mqtt_dotdot_user_credential_delete_user_callback_set(
    &delete_user_command);
  // Credential
  uic_mqtt_dotdot_user_credential_add_credential_callback_set(
    &add_credential_command);
  uic_mqtt_dotdot_user_credential_modify_credential_callback_set(
    &modify_credential_command);
  uic_mqtt_dotdot_user_credential_delete_credential_callback_set(
    &delete_credential_command);

  return SL_STATUS_OK;
}