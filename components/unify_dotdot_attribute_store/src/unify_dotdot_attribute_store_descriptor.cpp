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
// Include from this component
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_descriptor.h"
#include "unify_dotdot_attribute_store_configuration.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "uic_attribute_definitions.h"
#include "dotdot_mqtt_helpers.hpp"

// Include from other components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "sl_log.h"
#include "sl_status.h"
#include "uic_mqtt.h"
#include "attribute.hpp"
#include "dotdot_mqtt.h"
#include "process.h"

// Generic include
#include <stddef.h>
#include <string>
#include <iomanip>
#include <nlohmann/json.hpp>

using namespace attribute_store;
constexpr char LOG_TAG[] = "unify_dotdot_attribute_store_descriptor";

#define DESCRIPTOR(attribute) DOTDOT_ATTRIBUTE_ID_DESCRIPTOR_##attribute

static void publish_device_type_list(attribute_store_node_t attribute_id_node)
{
  dotdot_endpoint_id_t ep = 0;
  nlohmann::json jsn;
  std::string unid(MAXIMUM_UNID_SIZE, '\0');
  attribute_store_node_t endpoint_id_node
    = attribute_store_get_first_parent_with_type(attribute_id_node,
                                      unify_dotdot_attributes_endpoint_attribute());

  if (SL_STATUS_OK
      != unify_dotdot_attributes_get_unid_endpoint()(endpoint_id_node, &unid[0], &ep)) {
    sl_log_warning(LOG_TAG,
                   "Cannot derive UNID/EP from Attribute ID %d. "
                   "Node state will not be published.",
                   endpoint_id_node);
    return;
  }
  
  std::string topic = "ucl/by-unid/";
  topic.append(unid.c_str());
  topic += "/ep" + std::to_string(ep);

  // Get device type lists from attribute store
  uint8_t buffer[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  uint8_t size;
  attribute_store_get_node_attribute_value(attribute_id_node, REPORTED_ATTRIBUTE,
                                                                  buffer, &size);
  uint8_t device_list_type_count = size/sizeof(DeviceTypeStruct);
  DeviceTypeStruct* deviceTypeList = reinterpret_cast<DeviceTypeStruct *>(buffer);
                                                 
  // This is a variable size array of the same known type.
  // Create an array under the value {"value":[]}
  jsn["value"] = nlohmann::json::array();

  for (uint8_t i = 0; i < device_list_type_count; i++) {
    // Struct type
    nlohmann::json json_object = nlohmann::json::object();
    json_object["DeviceType"] = dev_type_id_get_enum_value_name(deviceTypeList[i].DeviceType);
    json_object["Revision"] = deviceTypeList[i].Revision;
    jsn["value"].push_back(json_object);
  }

  std::string payload_str;
  try {
    // Payload contains data from end nodes, which we cannot control, thus we handle if there are non-utf8 characters
    payload_str = jsn.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);
  } catch (const nlohmann::json::exception& e) {
    sl_log_debug(LOG_TAG, "%s: Unable to parse JSON payload: %s", "Descriptor/Attributes/DeviceTypeList", e.what());
    return;
  }

  topic +=  "/Descriptor/Attributes/DeviceTypeList";
  std::string topic_desired = topic + "/Desired";
  uic_mqtt_publish(topic_desired.c_str(), payload_str.c_str(),
                  payload_str.length(), true);
  std::string topic_reported = topic + "/Reported";
  uic_mqtt_publish(topic_reported.c_str(), payload_str.c_str(),
                  payload_str.length(), true);
}

/**
 * @brief callback function for attributes that affect the device type list topic
 *
 * The following attributes will trigger this function
 * - DOTDOT_DESCRIPTOR_DEVICE_TYPE_LIST_ATTRIBUTE_ID
 * Any update will trigger a new MQTT publication.
 * 
 * @param updated_node Updated attribute store node
 * @param change       Type of change applied
 */
static void on_descriptor_device_type_attribute_update(
            attribute_store_node_t updated_node, attribute_store_change_t change)
{
  // Unretain of messages will be done in ZAP generated code for attribute deletion.
  if (change == ATTRIBUTE_DELETED) {
    return;
  }
  
  if (attribute_store_is_reported_defined(updated_node)) {
    attribute_store_set_desired_as_reported(updated_node);
    publish_device_type_list(updated_node);
  }
}

sl_status_t unify_dotdot_attribute_store_descriptor_init()
{
  attribute_store_register_callback_by_type_and_state(
    &on_descriptor_device_type_attribute_update,
    DESCRIPTOR(DEVICE_TYPE_LIST),
    REPORTED_ATTRIBUTE);
  
  return SL_STATUS_OK;
}

