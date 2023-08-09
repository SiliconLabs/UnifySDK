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
#include "on_off_cluster_basic_mapper.h"

// Includes from Unify
#include "sl_log.h"
#include "sl_status.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"

#include "unify_dotdot_defined_attribute_types.h"
#include "unify_dotdot_attribute_store_node_state.h"
// Includes from auto-generated files
#include "dotdot_mqtt.h"

// Setup Log ID
constexpr char LOG_TAG[] = "on_off_cluster_basic_mapper";

///////////////////////////////////////////////////////////////////////////////
// Control functions
///////////////////////////////////////////////////////////////////////////////
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
  NodeStateNetworkStatus network_status
    = unify_attribute_store_node_state_get_status(node);
  if (network_status != ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL) {
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
bool on_off_cluster_basic_mapper_init()
{
  sl_log_debug(LOG_TAG, "OnOff cluster (Basic) mapper initialization");

  // Listen to the BASIC Value attribute is created
  attribute_store_register_callback_by_type(
    on_onoff_attribute_created_due_to_basic_value,
    DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF);

  return true;
}
