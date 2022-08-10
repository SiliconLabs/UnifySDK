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
#include "on_off_cluster_mapper.h"
#include "dotdot_attributes.h"

// Includes from Unify
#include "sl_log.h"
#include "sl_status.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"
#include "ucl_definitions.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"

// Setup Log ID
#define LOG_TAG "on_off_cluster_mapper"

///////////////////////////////////////////////////////////////////////////////
// Control functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t
  on_off_cluster_mapper_on(const dotdot_unid_t unid,
                           const dotdot_endpoint_id_t endpoint,
                           uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (callback_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return dotdot_is_supported_on_off_on_off(unid, endpoint) ? SL_STATUS_OK
                                                             : SL_STATUS_FAIL;
  }
  return dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, true);
}

sl_status_t
  on_off_cluster_mapper_off(const dotdot_unid_t unid,
                            const dotdot_endpoint_id_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (callback_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return dotdot_is_supported_on_off_on_off(unid, endpoint) ? SL_STATUS_OK
                                                             : SL_STATUS_FAIL;
  }

  return dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, false);
}

sl_status_t on_off_cluster_mapper_toggle(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (callback_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return dotdot_is_supported_on_off_on_off(unid, endpoint) ? SL_STATUS_OK
                                                             : SL_STATUS_FAIL;
  }
  bool set_value
    = !dotdot_get_on_off_on_off(unid, endpoint, REPORTED_ATTRIBUTE);

  return dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, set_value);
}

// The basic value is usually created after the node interview is done.
// Therefore, we fist check the node interview is done. Then, we check
// if the corrsponding dotdot attribute is created if it does we republish
// the supported commands to enure that OnOff cluster is publish as supported
// one a given node is only supporting Basic command class and it maped to
// dotdot OnOff cluster attribute.
static void
  on_onoff_attribute_created_due_to_basic_value(attribute_store_node_t node,
                                                attribute_store_change_t change)
{
  if (change != ATTRIBUTE_CREATED) {
    return;
  }
  node_state_topic_state_t network_status
    = attribute_store_network_helper_get_network_status(node);
  if (network_status != NODE_STATE_TOPIC_STATE_INCLUDED) {
    return;
  }
  unid_t unid;
  zwave_endpoint_id_t endpoint_id;
  attribute_store_network_helper_get_unid_endpoint_from_node(node,
                                                             unid,
                                                             &endpoint_id);
  uic_mqtt_dotdot_publish_supported_commands(unid, endpoint_id);
}
///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
bool on_off_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "OnOff cluster mapper initialization");

  uic_mqtt_dotdot_on_off_on_callback_set(on_off_cluster_mapper_on);
  uic_mqtt_dotdot_on_off_off_callback_set(on_off_cluster_mapper_off);
  uic_mqtt_dotdot_on_off_toggle_callback_set(on_off_cluster_mapper_toggle);

  // Listen to the BASIC Value attribute is created
  attribute_store_register_callback_by_type(
    on_onoff_attribute_created_due_to_basic_value,
    DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF);

  return true;
}

int on_off_cluster_mapper_teardown()
{
  sl_log_debug(LOG_TAG, "Teardown of the OnOff cluster mapper");
  return 0;
}
