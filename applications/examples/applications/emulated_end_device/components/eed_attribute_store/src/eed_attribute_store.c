/* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "eed_attribute_store.h"
#include "eed_attribute_store_helper.h"
#include "eed_attribute_store_type_registration.h"
#include "eed_attribute_store_attribute_types.h"
#include "eed_attribute_store_clusters.h"
#include "eed_dotdot_create_clusters.h"

// Generic includes
#include <string.h>

// Unify Components
#include "attribute_store_helper.h"
#include "attribute_store_configuration.h"
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "unify_dotdot_attribute_store_node_state.h"
#include "attribute_timeouts.h"

#include "sl_log.h"
#define LOG_TAG "eed_attribute_store"
// Time that it takes to add a cluster, in seconds
#define INTERVIEW_TIME 3 * 1000

// EED Configuration for the Unify DotDot Attribute Store
const unify_dotdot_attribute_store_configuration_t eed_configuration = {
  .get_endpoint_node_function = &eed_attribute_store_get_endpoint_node,
  .get_unid_endpoint_function = &eed_attribute_store_get_endpoint_from_node,
  .get_unid_function          = &eed_attribute_store_get_unid_from_node,
  .update_attribute_desired_values_on_commands = true,
  .clear_reported_on_desired_updates           = false,
  .automatic_deduction_of_supported_commands   = true,
  .force_read_attributes_enabled               = true,
  .write_attributes_enabled                    = true,
  .publish_desired_attribute_values_to_mqtt    = true,
  .publish_reported_attribute_values_to_mqtt   = true,
  .node_type                                   = ATTRIBUTE_UNID,
  .endpoint_type                               = ATTRIBUTE_ENDPOINT_ID,
};

static void return_to_online_functional(attribute_store_node_t endpoint_node)
{
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_node_t unid_node
      = attribute_store_get_first_parent_with_type(endpoint_node,
                                                   ATTRIBUTE_UNID);
  attribute_store_set_child_reported(unid_node,
                                     DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
  attribute_store_set_child_desired(unid_node,
                                    DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                    &network_status,
                                    sizeof(network_status));
}

sl_status_t eed_attribute_store_add_cluster_to_endpoint(dotdot_unid_t unid, dotdot_endpoint_id_t endpoint_id,
  const char* cluster_name) {
  sl_status_t status;
  attribute_store_node_t unid_node = eed_attribute_store_get_unid_node(unid);
  attribute_store_node_t endpoint_node = eed_attribute_store_get_endpoint_node(unid, endpoint_id);
  const NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING;
  status = attribute_store_set_child_desired(unid_node,
                                    DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                    &network_status,
                                    sizeof(network_status));
  status = attribute_store_set_child_reported(unid_node,
                                     DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
  attribute_timeout_set_callback(endpoint_node,
                                 INTERVIEW_TIME,
                                 &return_to_online_functional);

  eed_dotdot_create_cluster_by_name(unid, endpoint_id, cluster_name);

  return status;
}

sl_status_t eed_attribute_store_remove_cluster_on_endpoint(dotdot_unid_t unid, dotdot_endpoint_id_t endpoint_id,
  const char* cluster_name) {
  sl_status_t status;
  attribute_store_node_t unid_node = eed_attribute_store_get_unid_node(unid);
  attribute_store_node_t endpoint_node = eed_attribute_store_get_endpoint_node(unid, endpoint_id);
  const NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING;
  status = attribute_store_set_child_desired(unid_node,
                                  DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                  &network_status,
                                  sizeof(network_status));
  status = attribute_store_set_child_reported(unid_node,
                                     DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
  attribute_timeout_set_callback(endpoint_node,
                                 INTERVIEW_TIME,
                                 &return_to_online_functional);

  eed_dotdot_unretain_cluster_by_name(unid, endpoint_id, cluster_name);

  return status;
}

sl_status_t eed_attribute_store_init()
{
  sl_status_t status = eed_attribute_store_register_known_attribute_types();

  // Configure the attribute store.
  // Save in the worst case scenario every 10 minutes if we keep changing the
  // Attribute Store aggressively
  attribute_store_configuration_set_auto_save_safety_interval(10 * 60);
  // Wait 10 seconds since the last Attribute Store change before saving
  // to the datastore.
  attribute_store_configuration_set_auto_save_cooldown_interval(10);
  attribute_store_configuration_set_type_validation(true);

  // Configure the Unify DotDot Attribute Store component:
  unify_dotdot_attribute_store_set_configuration(&eed_configuration);

  // Get the cluster deamon ready
  status |= eed_attribute_store_clusters_init();

  // Get the cluster command handler ready
  status |= eed_attribute_store_command_handler_init();

  // Custom setup of attribute values
  status |= eed_attribute_store_attribute_custom_handler_init();

  // Just simulate an update of the whole Attribute Store.
  status |= attribute_store_refresh_node_and_children_callbacks(
    attribute_store_get_root());
  return status;
}
