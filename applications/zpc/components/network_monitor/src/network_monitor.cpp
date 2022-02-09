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
// Generic includes
#include <cstddef>
#include <cstring>
#include <string>
#include <map>
#include <memory>

// Includes from this component
#include "network_monitor.h"
#include "network_monitor_span_persistence.h"

// Interfaces
#include "ucl_definitions.h"
#include "zwave_command_class_wake_up_types.h"

// Includes from other components
#include "zpc_endian.h"
#include "zwave_api.h"
#include "zwave_utils.h"
#include "zwave_network_management.h"
#include "zwave_network_management_state.h"
#include "zwave_controller.h"
#include "zwave_controller_keyset.h"
#include "zwave_controller_utils.h"
#include "zwave_command_classes_fixt.h"
#include "zwave_unid.h"
#include "sl_log.h"
#include "zwave_tx_scheme_selector.h"
#include "uic_mqtt.h"
#include "zpc_converters.h"
#include "sys/clock.h"
#include "sys/ctimer.h"
#include "zpc_config.h"
#include "ucl_mqtt_node_interview.h"
#include "zwave_controller_storage.h"
#include "zcl_cluster_servers.h"

#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"

#include "attribute.hpp"
#include "failing_node_monitor.h"

// Setup the logging
#define LOG_TAG "network_monitor"

using namespace attribute_store;

/**
 * @brief @ref Contiki Event definitions for the Network Monitor Process.
 */
typedef enum {
  /// The ZPC entered a new network
  NEW_NETWORK_EVENT,
  /// The ZPC received a new NodeID for itself
  NODE_ID_ASSIGNED_EVENT,
  /// A new node was added to the network
  NODE_ADDED_EVENT,
  /// A node interview has been initiated
  NODE_INTERVIEW_INITIATED_EVENT,
  /// Node interview done
  NODE_INTERVIEW_DONE_EVENT,
  /// An existing node was removed from the network
  NODE_DELETED_EVENT,
  /// Frame transmission failed
  NODE_FRAME_TRANSMISSION_FAILED_EVENT,
  /// successful Frame Transmission
  NODE_FRAME_TRANSMISSION_SUCCESS_EVENT,
  /// Frame Rx
  NODE_FRAME_RX_EVENT
} network_monitor_events_t;

/**
 * @brief Struct used for sending event data with event \ref NEW_NETWORK_EVENT.
 */
struct new_network_entered_data {
  zwave_home_id_t home_id;              ///< Home ID
  zwave_home_id_t node_id;              ///< Node ID
  zwave_keyset_t granted_keys;          ///< Granted Keys
  zwave_kex_fail_type_t kex_fail_type;  ///< Kex Fail type.
  bool remove_old_network;              ///< Set to true to remove old network
};

/**
 * @brief Struct used for sending event data with event \ref NODE_ADDED_EVENT.
 */
struct node_added_event_data {
  zwave_node_info_t nif;                ///< NIF
  zwave_node_id_t node_id;              ///< Node ID
  zwave_dsk_t dsk;                      ///< DSK
  zwave_keyset_t granted_keys;          ///< Granted Keys
  zwave_kex_fail_type_t kex_fail_type;  ///< Kex Fail type
  zwave_protocol_t inclusion_protocol;
};

/**
 * @brief Struct used for sending event data with event \ref NODE_ID_ASSIGNED_EVENT.
 */
struct node_id_assigned_event_data {
  zwave_node_id_t node_id;  ///< Node ID
  zwave_protocol_t inclusion_protocol;
};

struct network_monitor_sleeping_node_info {
  struct ctimer timer;
  wake_up_interval_t wakeup_interval;
  zwave_node_id_t node_id;
  bool
    network_status;  // Status that represents the availability of the sleeping node
                     // if the node is not responding this flag shall be false.

  ~network_monitor_sleeping_node_info()
  {
    //make sure there is no running timers on this object when its deleted
    ctimer_stop(&timer);
  }
};

static std::map<zwave_node_id_t, uint8_t> failed_transmission_data;
static std::map<zwave_node_id_t,
                std::shared_ptr<network_monitor_sleeping_node_info>>
  sleeping_nodes_status;

// Private variables
static unid_t zpc_unid;
static zwave_nodemask_t current_node_list;

// Forward declarations
static void
  network_monitor_on_node_id_assigned(zwave_node_id_t node_id,
                                      bool included_by_us,
                                      zwave_protocol_t inclusion_protocol);
static void network_monitor_on_node_deleted(zwave_node_id_t node_id);
static void network_monitor_on_node_added(sl_status_t status,
                                          const zwave_node_info_t *nif,
                                          zwave_node_id_t node_id,
                                          zwave_dsk_t dsk,
                                          zwave_keyset_t granted_keys,
                                          zwave_kex_fail_type_t kex_fail_type,
                                          zwave_protocol_t inclusion_protocol);
static void network_monitor_on_nif_updated(attribute_store_node_t updated_node,
                                           attribute_store_change_t change);

static void
  network_monitor_on_new_network_entered(zwave_home_id_t __home_id,
                                         zwave_node_id_t __node_id,
                                         zwave_keyset_t __granted_keys,
                                         zwave_kex_fail_type_t kex_fail_type);

static attribute network_monitor_add_attribute_store_node(
  zwave_node_id_t node_id, node_state_topic_state_t network_status);
static void
  network_monitor_remove_attribute_store_node(zwave_node_id_t node_id);
static void
  network_monitor_activate_network_resolution(zwave_home_id_t current_home_id);

static void
  network_monitor_on_frame_transmission_failed(zwave_node_id_t node_id);

static void
  network_monitor_on_frame_transmission_success(zwave_node_id_t node_id);

static void network_monitor_wakeup_interval_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change);

static void
  network_monitor_sleeping_nodes_network_status_controller(void *data);

static void network_monitor_on_frame_received(zwave_node_id_t node_id);

static void network_monitor_handle_event_success_frame_transmission(
  zwave_node_id_t node_id);

static void
  network_monitor_on_frame_transmission(bool transmission_successful,
                                        const zwapi_tx_report_t *tx_report,
                                        zwave_node_id_t node_id);

static zwave_controller_callbacks_t network_monitor_callbacks
  = {.on_node_id_assigned    = &network_monitor_on_node_id_assigned,
     .on_node_deleted        = &network_monitor_on_node_deleted,
     .on_node_added          = &network_monitor_on_node_added,
     .on_new_network_entered = &network_monitor_on_new_network_entered,
     .on_frame_transmission  = &network_monitor_on_frame_transmission,
     .on_rx_frame_received   = &network_monitor_on_frame_received};

static void
  network_monitor_on_frame_transmission(bool transmission_successful,
                                        const zwapi_tx_report_t *tx_report,
                                        zwave_node_id_t node_id)
{
  if (transmission_successful) {
    network_monitor_on_frame_transmission_success(node_id);
  } else {
    network_monitor_on_frame_transmission_failed(node_id);
  }
}

static attribute_store_node_t
  attribute_store_node_from_zwave_node(zwave_node_id_t node_id)
{
  // Convert our node_id to a UNID:
  unid_t unid;
  zwave_unid_from_node_id(node_id, unid);
  // Find out attribute store node based on the UNID
  return attribute_store_network_helper_get_node_id_node(unid);
}

static void
  network_monitor_on_node_id_assigned(zwave_node_id_t node_id,
                                      bool included_by_us,
                                      zwave_protocol_t inclusion_protocol)
{
  struct node_id_assigned_event_data *event_data
    = new struct node_id_assigned_event_data;
  // Copy all the data about this new added node.
  event_data->node_id            = node_id;
  event_data->inclusion_protocol = inclusion_protocol;

  process_post(&network_monitor_process, NODE_ID_ASSIGNED_EVENT, event_data);
}

static void network_monitor_on_nif_updated(attribute_store_node_t updated_node,
                                           attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (true
      == attribute_store_is_value_defined(updated_node, REPORTED_ATTRIBUTE)) {
    return;
  }

  // Fresh NIF created/updated but still undefined, it means we are interviewing.
  attribute attr_updated_node(updated_node);
  attribute attr_node_id = attr_updated_node.first_parent(ATTRIBUTE_NODE_ID);
  process_post(&network_monitor_process,
               NODE_INTERVIEW_INITIATED_EVENT,
               (void *)(intptr_t)attr_node_id);
}

static void network_monitor_on_node_deleted(zwave_node_id_t node_id)
{
  process_post(&network_monitor_process,
               NODE_DELETED_EVENT,
               (void *)(intptr_t)node_id);
}

static void network_monitor_on_node_added(sl_status_t status,
                                          const zwave_node_info_t *nif,
                                          zwave_node_id_t node_id,
                                          zwave_dsk_t dsk,
                                          zwave_keyset_t granted_keys,
                                          zwave_kex_fail_type_t kex_fail_type,
                                          zwave_protocol_t inclusion_protocol)
{
  struct node_added_event_data *event_data = new struct node_added_event_data;
  // Copy all the data about this new added node.
  memcpy(event_data->dsk, dsk, sizeof(zwave_dsk_t));
  memcpy(&(event_data->nif), nif, sizeof(zwave_node_info_t));
  event_data->node_id            = node_id;
  event_data->granted_keys       = granted_keys;
  event_data->kex_fail_type      = kex_fail_type;
  event_data->inclusion_protocol = inclusion_protocol;

  process_post(&network_monitor_process, NODE_ADDED_EVENT, event_data);
}

static void
  network_monitor_on_new_network_entered(zwave_home_id_t home_id,
                                         zwave_node_id_t node_id,
                                         zwave_keyset_t granted_keys,
                                         zwave_kex_fail_type_t kex_fail_type)
{
  new_network_entered_data *data = new new_network_entered_data;
  data->node_id                  = node_id;
  data->home_id                  = home_id;
  data->granted_keys             = granted_keys;
  data->remove_old_network       = true;
  process_post(&network_monitor_process, NEW_NETWORK_EVENT, data);
}

static void
  network_monitor_node_id_resolution_listener(attribute_store_node_t node)
{
  process_post(&network_monitor_process,
               NODE_INTERVIEW_DONE_EVENT,
               reinterpret_cast<void *>(static_cast<intptr_t>(node)));
}

static void
  network_monitor_on_frame_transmission_failed(zwave_node_id_t node_id)
{
  process_post(&network_monitor_process,
               NODE_FRAME_TRANSMISSION_FAILED_EVENT,
               (void *)(intptr_t)node_id);
}

static void
  network_monitor_on_frame_transmission_success(zwave_node_id_t node_id)
{
  process_post(&network_monitor_process,
               NODE_FRAME_TRANSMISSION_SUCCESS_EVENT,
               (void *)(intptr_t)node_id);
}

// On frame received callback handler
static void network_monitor_on_frame_received(zwave_node_id_t node_id)
{
  // ZPC received a frame from a given node. If this node is in the failing list,
  // we remove the node from failing list
  process_post(&network_monitor_process,
               NODE_FRAME_RX_EVENT,
               (void *)(intptr_t)node_id);
}

/**
 * @brief Create nodes in the attribute store for all
 * nodes currently in the network
 *
 * This function gets the list of nodes from @ref zwapi
 * and populates the attribute store tree.
 */
static void network_monitor_create_attribute_store_network_nodes(
  zwave_keyset_t granted_keys)
{
  if (SL_STATUS_OK != zwapi_get_full_node_list(current_node_list)) {
    return;
  }
  for (zwave_node_id_t node_id = ZW_MIN_NODE_ID; node_id <= ZW_LR_MAX_NODE_ID;
       node_id++) {
    if (!ZW_IS_NODE_IN_MASK(node_id, current_node_list)) {
      continue;
    }
    const bool zpc_node = node_id == zwave_network_management_get_node_id();
    // Create the node, set NODE_STATE_TOPIC_STATE_INCLUDED for ZPC node,
    // NODE_STATE_TOPIC_STATE_NODEID_ASSIGNED for all end devices
    attribute attr_node_id_node = network_monitor_add_attribute_store_node(
      node_id,
      zpc_node ? NODE_STATE_TOPIC_STATE_INCLUDED
               : NODE_STATE_TOPIC_STATE_NODEID_ASSIGNED);
    // If it's our own NodeID, make sure to have our granted keys saved
    if (zpc_node) {
      // Find if we have granted keys under our node_id_node
      if (attr_node_id_node.child_by_type(ATTRIBUTE_GRANTED_SECURITY_KEYS, 0)
          == ATTRIBUTE_STORE_INVALID_NODE) {
        attr_node_id_node.add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS)
          .set_reported<zwave_keyset_t>(granted_keys);
      }
    } else {
      // Create the non-secure NIF attribute under EP0 if it is missing
      attribute attr_endpoint0
        = attr_node_id_node.child_by_type(ATTRIBUTE_ENDPOINT_ID, 0);
      if (attr_endpoint0.child_by_type(ATTRIBUTE_ZWAVE_NIF, 0)
          == ATTRIBUTE_STORE_INVALID_NODE)
        attr_endpoint0.add_node(ATTRIBUTE_ZWAVE_NIF);
      attribute_resolver_set_resolution_listener(
        attr_node_id_node,
        network_monitor_node_id_resolution_listener);
    }
  }
}

static void network_monitor_pause_nl_nodes_resolution(
  attribute_store_node_t current_network_node)
{
  uint32_t node_id_node_index = 0;
  attribute_store_node_t node_id_node
    = attribute_store_get_node_child_by_type(current_network_node,
                                             ATTRIBUTE_NODE_ID,
                                             node_id_node_index);
  zwave_node_id_t node_id = 0;
  while (ATTRIBUTE_STORE_INVALID_NODE != node_id_node) {
    node_id = 0;
    attribute_store_read_value(node_id_node,
                               REPORTED_ATTRIBUTE,
                               &node_id,
                               sizeof(zwave_node_id_t));
    if (OPERATING_MODE_NL == zwave_get_operating_mode(node_id)) {
      sl_log_debug(LOG_TAG,
                   "Pausing attribute resolution for NL node: NodeID %d",
                   node_id);
      attribute_resolver_pause_node_resolution(node_id_node);
    }
    node_id_node_index++;
    node_id_node = attribute_store_get_node_child_by_type(current_network_node,
                                                          ATTRIBUTE_NODE_ID,
                                                          node_id_node_index);
  }
}

static void
  network_monitor_activate_network_resolution(zwave_home_id_t current_home_id)
{
  uint8_t home_id_node_index  = 0;
  attribute_store_node_t root = attribute_store_get_root();
  attribute_store_node_t current_network_node
    = attribute_store_get_node_child_by_value(root,
                                              ATTRIBUTE_HOME_ID,
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&current_home_id,
                                              sizeof(current_home_id),
                                              0);
  attribute_store_node_t network_node
    = attribute_store_get_node_child_by_type(root,
                                             ATTRIBUTE_HOME_ID,
                                             home_id_node_index);
  home_id_node_index++;

  // Pause the resolutions for all foreign networks
  while (ATTRIBUTE_STORE_INVALID_NODE != network_node) {
    if (network_node != current_network_node) {
      attribute_resolver_pause_node_resolution(network_node);
      sl_log_debug(
        LOG_TAG,
        "Pausing foreign HomeID Network resolution. (Attribute ID %d)",
        network_node);
    }
    network_node = attribute_store_get_node_child_by_type(root,
                                                          ATTRIBUTE_HOME_ID,
                                                          home_id_node_index);
    home_id_node_index++;
  }

  // Then look at our network... But before we enable resolution, ensure
  // that NL nodes are paused, else we will send commands to sleeping nodes
  network_monitor_pause_nl_nodes_resolution(current_network_node);
  // Activate the resolution for our network only:
  attribute_resolver_resume_node_resolution(current_network_node);
}

/**
 * @brief Removes a given HomeID from the attribute store.
 *
 * @param unid  The @ref unid_t of the node for which the HomeID
 *              node will be extracted and then removed from the
 *              attribute store.
 */
static void network_monitor_remove_attribute_store_home_id(unid_t old_unid)
{
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(old_unid, &home_id);
  sl_log_debug(LOG_TAG,
               "Removing HomeID %08X from the Attribute Store.",
               home_id);

  attribute_store_node_t home_id_node
    = attribute_store_network_helper_get_home_id_node(old_unid);

  // Delete the node
  attribute_store_delete_node(home_id_node);
}

/**
 * @brief Removes a given NodeID from the attribute store.
 *
 * @param node_id The @ref zwave_node_id_t of the node
 *                to remove from the attribute store.
 */
static void network_monitor_remove_attribute_store_node(zwave_node_id_t node_id)
{
  sl_log_debug(LOG_TAG,
               "Removing NodeID %d from the Attribute Store.",
               node_id);
  // Find out attribute store node based on the zwave_node_id_t
  attribute_store_node_t node_id_node
    = attribute_store_node_from_zwave_node(node_id);

  // Clear resolution listener on the node
  attribute_resolver_clear_resolution_listener(
    node_id_node,
    network_monitor_node_id_resolution_listener);
  // Delete the node
  attribute_store_delete_node(node_id_node);
}

/**
 * @brief Adds a given NodeID to the attribute store.
 *
 * @param node_id The @ref zwave_node_id_t of the node
 *                to add in the attribute store.
 */
static attribute network_monitor_add_attribute_store_node(
  zwave_node_id_t node_id, node_state_topic_state_t network_status)
{
  sl_log_debug(
    LOG_TAG,
    "Making sure that NodeID %d (with endpoint 0) is in the Attribute Store.",
    node_id);
  unid_t unid;
  zwave_unid_from_node_id(node_id, unid);
  attribute attr_node_id_node(
    attribute_store_network_helper_create_node_id_node(unid));
  if (attr_node_id_node.child_by_type(ATTRIBUTE_ENDPOINT_ID, 0)
      == static_cast<attribute_store_node_t>(ATTRIBUTE_STORE_INVALID_NODE)) {
    attr_node_id_node.add_node(ATTRIBUTE_ENDPOINT_ID)
      .set_reported<zwave_endpoint_id_t>(0);
  }
  if (attr_node_id_node.child_by_type(ATTRIBUTE_NETWORK_STATUS, 0)
      == static_cast<attribute_store_node_t>(ATTRIBUTE_STORE_INVALID_NODE)) {
    attr_node_id_node.add_node(ATTRIBUTE_NETWORK_STATUS)
      .set_reported<node_state_topic_state_t>(network_status);
  }
  return attr_node_id_node;
}

/**
 * @brief Copies all the inclusion data for a newly added Node to the attribute store
 *
 * @param node_id The @ref zwave_node_id_t of the node
 *                that was just included
 */
static void network_monitor_update_new_node_attribute_store(
  const node_added_event_data &node_added_data)
{
  // Find the node from the attribute store:
  unid_t unid;
  zwave_unid_from_node_id(node_added_data.node_id, unid);

  // Find out attribute store node based on the UNID
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_create_node_id_node(unid);

  // Find the granted keys for that node.
  attribute_store_node_t new_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                             0);

  if (new_node == ATTRIBUTE_STORE_INVALID_NODE) {
    new_node
      = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);
  }

  // Write the granted keys
  attribute_store_set_node_attribute_value(new_node,
                                           REPORTED_ATTRIBUTE,
                                           &node_added_data.granted_keys,
                                           sizeof(zwave_keyset_t));

  // Find the KEX Fail type for that node
  new_node = attribute_store_get_node_child_by_type(node_id_node,
                                                    ATTRIBUTE_KEX_FAIL_TYPE,
                                                    0);
  if (new_node == ATTRIBUTE_STORE_INVALID_NODE) {
    new_node = attribute_store_add_node(ATTRIBUTE_KEX_FAIL_TYPE, node_id_node);
  }

  // Write the KEX Fail type
  attribute_store_set_node_attribute_value(
    new_node,
    REPORTED_ATTRIBUTE,
    reinterpret_cast<const uint8_t *>(&node_added_data.kex_fail_type),
    sizeof(uint8_t));

  // Find the DSK for that node
  new_node
    = attribute_store_get_node_child_by_type(node_id_node, ATTRIBUTE_S2_DSK, 0);
  if (new_node == ATTRIBUTE_STORE_INVALID_NODE) {
    new_node = attribute_store_add_node(ATTRIBUTE_S2_DSK, node_id_node);
  }

  // Write the S2 DSK for that node
  attribute_store_set_node_attribute_value(new_node,
                                           REPORTED_ATTRIBUTE,
                                           node_added_data.dsk,
                                           sizeof(zwave_dsk_t));

  // Find the protocol listening byte from the NIF
  new_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                             0);
  if (new_node == ATTRIBUTE_STORE_INVALID_NODE) {
    new_node = attribute_store_add_node(ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                        node_id_node);
  }
  // Write the protocol listening byte from the NIF
  attribute_store_set_node_attribute_value(
    new_node,
    REPORTED_ATTRIBUTE,
    &node_added_data.nif.listening_protocol,
    sizeof(node_added_data.nif.listening_protocol));

  // Find the optional protocol byte from the NIF
  new_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                             0);
  if (new_node == ATTRIBUTE_STORE_INVALID_NODE) {
    new_node = attribute_store_add_node(ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                        node_id_node);
  }
  // Write the protocol listening byte from the NIF
  attribute_store_set_node_attribute_value(
    new_node,
    REPORTED_ATTRIBUTE,
    &node_added_data.nif.optional_protocol,
    sizeof(node_added_data.nif.optional_protocol));

  // Find the Non-secure NIF for the node/endpoint 0
  attribute_store_node_t endpoint_id_node
    = attribute_store_network_helper_get_endpoint_node(unid, 0);
  new_node = attribute_store_get_node_child_by_type(endpoint_id_node,
                                                    ATTRIBUTE_ZWAVE_NIF,
                                                    0);
  if (new_node == ATTRIBUTE_STORE_INVALID_NODE) {
    new_node = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  }
}

// Handler Functions for events

static void
  network_monitor_handle_event_new_network(new_network_entered_data *event_data)
{
  // If we just started, do not try to clean up the old network
  if (event_data->remove_old_network) {
    // Delete the old HomeID node from the attribute store, using the old HomeID
    network_monitor_remove_attribute_store_home_id(zpc_unid);
  }
  // Configure our new HomeID
  zwave_unid_set_home_id(event_data->home_id);
  // Populate the Attribute Store: Important to do it before MQTT
  // as MQTT publish functions read information from the Attribute Store
  network_monitor_create_attribute_store_network_nodes(
    event_data->granted_keys);
  // Pause node resolution on any other network than ours in the Attribute Store.
  network_monitor_activate_network_resolution(event_data->home_id);
  //set the zpc unid
  zwave_unid_from_node_id(event_data->node_id, zpc_unid);

  if (event_data->remove_old_network) {
    // Reinitialize our command class handlers, it we entered a new network.
    // This is for our supported CCs that write in the attribute store under the ZPC node.
    zwave_command_classes_init();
    // Tell the ZCL Cluster servers to init in our new network.
    zcl_cluster_servers_init();
  }

  delete event_data;
  // Cleaning data structures that contains the zwave_node_id key
  failed_transmission_data.clear();
  sleeping_nodes_status.clear();
  failing_node_monitor_list_clear();
}

static void network_monitor_handle_event_node_id_assigned(
  node_id_assigned_event_data *event_data)
{
  network_monitor_add_attribute_store_node(
    event_data->node_id,
    NODE_STATE_TOPIC_STATE_NODEID_ASSIGNED);
  zwave_store_inclusion_protocol(event_data->node_id,
                                 event_data->inclusion_protocol);
}

static void
  network_monitor_handle_event_node_added(node_added_event_data *event_data)
{
  // Attribute store node should already exist, but in case NODE_ID_ASSIGNED_EVENT
  // did not happen before this event, we ensure the node exists in the attribute store.
  network_monitor_update_new_node_attribute_store(*event_data);

  zwave_store_inclusion_protocol(event_data->node_id,
                                 event_data->inclusion_protocol);

  // Finally we want to update our local cache of the node list:
  zwapi_get_full_node_list(current_node_list);
  // TODO: Add timeout system to detect node interview failed
  delete event_data;
}

static void network_monitor_handle_event_node_interview_initiated(
  attribute_store_node_t node_id_node)
{
  attribute att(node_id_node);
  attribute network_status = att.child_by_type(ATTRIBUTE_NETWORK_STATUS);
  if (network_status.is_valid()) {
    try {
      node_state_topic_state_t node_state
        = network_status.reported<node_state_topic_state_t>();
      if (node_state == NODE_STATE_TOPIC_STATE_OFFLINE) {
        network_status.set_reported<node_state_topic_state_t>(
          NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL);
      } else {
        network_status.set_reported<node_state_topic_state_t>(
          NODE_STATE_TOPIC_INTERVIEWING);
      }
    } catch (std::exception &ex) {
      network_status.set_reported<node_state_topic_state_t>(
        NODE_STATE_TOPIC_INTERVIEWING);
      sl_log_warning(LOG_TAG,
                     "Cannot read the previous network status of a node. "
                     "Current status set to 'Interviewing' %s",
                     ex.what());
    }
  }

  // Register the listener for the node interview
  attribute_resolver_set_resolution_listener(
    att,
    network_monitor_node_id_resolution_listener);
}

static void network_monitor_handle_event_node_interview_done(
  attribute_store_node_t node_id_node)
{
  attribute att(node_id_node);
  attribute_resolver_clear_resolution_listener(
    node_id_node,
    network_monitor_node_id_resolution_listener);
  // update the NetworkStatus
  if (attribute network_status = att.child_by_type(ATTRIBUTE_NETWORK_STATUS);
      network_status.is_valid()) {
    network_status.set_reported<node_state_topic_state_t>(
      NODE_STATE_TOPIC_STATE_INCLUDED);
  }
}

static void
  update_last_received_frame_timestamp(attribute_store_node_t node_id_node)
{
  unsigned long current_time = clock_seconds();
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP,
                                     &current_time,
                                     sizeof(current_time));
}

static void network_monitor_handle_event_node_deleted(zwave_node_id_t node_id)
{
  if (node_id < ZW_MIN_NODE_ID) {
    return;
  }

  // Remove the node from the attribute store
  network_monitor_remove_attribute_store_node(node_id);
  // Cleaning data structures that contains the zwave_node_id key
  auto it_failed_transmission = failed_transmission_data.find(node_id);
  if (it_failed_transmission != failed_transmission_data.end()) {
    stop_monitoring_failing_node(node_id);
    failed_transmission_data.erase(it_failed_transmission);
  }
  auto it_sleeping_node = sleeping_nodes_status.find(node_id);
  if (it_sleeping_node != sleeping_nodes_status.end()) {
    sleeping_nodes_status.erase(it_sleeping_node);
  }
}

static void network_monitor_handle_event_failed_frame_transmission(
  zwave_node_id_t node_id)
{
  zwave_operating_mode_t operating_mode = zwave_get_operating_mode(node_id);

  // Look for the node in failed transmission 
  auto it = failed_transmission_data.find(node_id);
  // if the node was not in failed transmission list insert it and return
  if (it == failed_transmission_data.end()) {
    failed_transmission_data.insert(
      std::pair<zwave_node_id_t, uint8_t>(node_id, 1));
    return;
  }

  //Increase failure count
  it->second++;
  //If the failure count is not equal to accepted transmit failures, its ok, return
  if (it->second != zpc_get_config()->accepted_transmit_failure) {
    return;
  }

  attribute_store_node_t node_id_node
    = attribute_store_node_from_zwave_node(node_id);
  attribute attr(node_id_node);
  attribute network_status = attr.child_by_type(ATTRIBUTE_NETWORK_STATUS);
  if (!network_status.is_valid()) {
    return;
  }

  // If the network status was interviewing and the frame transmission is failed
  // the network status shall be NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL.
  if (network_status.reported<node_state_topic_state_t>()
      == NODE_STATE_TOPIC_INTERVIEWING) {
    network_status.set_reported<node_state_topic_state_t>(
      NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL);
  } else {
   // If the node is sleeping node, we should not update the network status
   // here since the sleeping node monitor does that using the wakeup period.
    if (OPERATING_MODE_NL != operating_mode) {
      sl_log_info(LOG_TAG,
                  "NodeID %d is now considered as failing/offline",
                  node_id);
      network_status.set_reported<node_state_topic_state_t>(
        NODE_STATE_TOPIC_STATE_OFFLINE);
    }
  }
  attribute_resolver_pause_node_resolution(node_id_node);
  // Failing node monitor does not monitor NL nodes
  if (OPERATING_MODE_NL != operating_mode) {
    start_monitoring_failing_node(node_id);
  }
}

static void network_monitor_handle_event_success_frame_transmission(
  zwave_node_id_t node_id)
{
  attribute_store_node_t node_id_node
    = attribute_store_node_from_zwave_node(node_id);
  // Save that we got a successful transmission.
  update_last_received_frame_timestamp(node_id_node);
  try {
    attribute attr(node_id_node);
    attribute network_status = attr.child_by_type(ATTRIBUTE_NETWORK_STATUS);
    unid_t unid;
    zwave_unid_from_node_id(node_id, unid);
    // Non-Sleeping nodes
    auto it = failed_transmission_data.find(node_id);
    if (it != failed_transmission_data.end()) {
      // If the network status was NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL,
      // we trigger a new interview (ucl_mqtt_initiate_node_interview()).
      if (network_status.reported<node_state_topic_state_t>()
          == NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL) {
        network_status.set_reported<node_state_topic_state_t>(
          NODE_STATE_TOPIC_INTERVIEWING);
        ucl_mqtt_initiate_node_interview(unid);
      } else {
        sl_log_info(LOG_TAG, "NodeID %d is online again", node_id);
        network_status.set_reported<node_state_topic_state_t>(
          NODE_STATE_TOPIC_STATE_INCLUDED);
      }
      attribute_resolver_resume_node_resolution(node_id_node);
      failed_transmission_data.erase(it);
      // Failing node monitor does not monitor NL nodes so no need of stopping
      if (OPERATING_MODE_NL != zwave_get_operating_mode(node_id)) {
        stop_monitoring_failing_node(node_id);
      }
    }

    // Sleeping node
    // Sleeping node may send other command first and then wakeup info
    // therefore we are handling the sleeping node successful transmission as
    // none sleeping nodes
    auto failing_list_it = sleeping_nodes_status.find(node_id);
    if (failing_list_it != sleeping_nodes_status.end()) {
      if (!(failing_list_it->second.get()->network_status)) {
        if (network_status.reported<node_state_topic_state_t>()
            == NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL) {
          network_status.set_reported<node_state_topic_state_t>(
            NODE_STATE_TOPIC_INTERVIEWING);
          ucl_mqtt_initiate_node_interview(unid);
        } else {
          sl_log_info(LOG_TAG, "NodeID %d is online again", node_id);
          network_status.set_reported<node_state_topic_state_t>(
            NODE_STATE_TOPIC_STATE_INCLUDED);
        }
        clock_time_t timeout = zpc_get_config()->missing_wake_up_notification
                               * failing_list_it->second.get()->wakeup_interval;
        failing_list_it->second.get()->network_status = true;
        ctimer_set(&failing_list_it->second.get()->timer,
                   timeout * CLOCK_SECOND,
                   network_monitor_sleeping_nodes_network_status_controller,
                   failing_list_it->second.get());
      }
    }
  } catch (std::exception &ex) {
    sl_log_warning(LOG_TAG,
                   "Cannot update network status for a sleeping node. %s",
                   ex.what());
  }
}

static void network_monitor_wakeup_interval_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  try {
    attribute attr(updated_node);
    wake_up_interval_t wakeup_interval = attr.reported<wake_up_interval_t>();
    // If the wakeup interval is set 0 means the node only be awake due to user action,
    // and we can not really monitor such nodes.
    if (wakeup_interval != 0) {
      attribute node_id_node = attr.first_parent(ATTRIBUTE_NODE_ID);
      std::shared_ptr<network_monitor_sleeping_node_info> wakeup_node_info
        = std::make_shared<network_monitor_sleeping_node_info>();
      wakeup_node_info->wakeup_interval = wakeup_interval;
      wakeup_node_info->node_id = node_id_node.reported<zwave_node_id_t>();
      wakeup_node_info->network_status = true;
      clock_time_t timeout = zpc_get_config()->missing_wake_up_notification
                             * wakeup_node_info->wakeup_interval;
      ctimer_set(&wakeup_node_info.get()->timer,
                 timeout * CLOCK_SECOND,
                 network_monitor_sleeping_nodes_network_status_controller,
                 wakeup_node_info.get());
      sleeping_nodes_status.try_emplace(wakeup_node_info->node_id,
                                        wakeup_node_info);
    }
  } catch (std::exception &ex) {
    return;
  }
}

static void network_monitor_on_wake_up_cc_version_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  try {
    attribute attr(updated_node);
    attribute node_id_node  = attr.first_parent(ATTRIBUTE_NODE_ID);
    zwave_node_id_t node_id = node_id_node.reported<zwave_node_id_t>();
    auto it_sleeping_node   = sleeping_nodes_status.find(node_id);
    if (it_sleeping_node == sleeping_nodes_status.end()) {
      std::shared_ptr<network_monitor_sleeping_node_info> wakeup_node_info
        = std::make_shared<network_monitor_sleeping_node_info>();
      wakeup_node_info->wakeup_interval = 0;
      wakeup_node_info->node_id         = node_id;
      wakeup_node_info->network_status  = true;
      sleeping_nodes_status.try_emplace(node_id, wakeup_node_info);
    }
  } catch (std::exception &ex) {
    sl_log_warning(LOG_TAG,
                   "Cannot read the Wake CC Version Attribute. %s",
                   ex.what());
  }
}

static void network_monitor_sleeping_nodes_network_status_controller(void *data)
{
  network_monitor_sleeping_node_info *read_data
    = static_cast<network_monitor_sleeping_node_info *>(data);
  if (read_data->network_status) {
    attribute node_id_node
      = attribute_store_node_from_zwave_node(read_data->node_id);
    attribute last_awaked_timestamp_node
      = node_id_node.child_by_type(ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP);
    unsigned long last_awake_timestamp = 0;
    if (SL_STATUS_OK
        != attribute_store_read_value(last_awaked_timestamp_node,
                                      REPORTED_ATTRIBUTE,
                                      &last_awake_timestamp,
                                      sizeof(last_awake_timestamp))) {
      unsigned long current_time_stamp = clock_seconds();
      unsigned long diff = current_time_stamp - last_awake_timestamp;
      if (diff >= (zpc_get_config()->missing_wake_up_notification
                   * read_data->wakeup_interval)) {
        // If the network status was interviewing and the frame transmission is failed
        // the network status shall be NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL.
        attribute network_status
          = node_id_node.child_by_type(ATTRIBUTE_NETWORK_STATUS);
        if (network_status.reported<node_state_topic_state_t>()
            == NODE_STATE_TOPIC_INTERVIEWING) {
          network_status.set_reported<node_state_topic_state_t>(
            NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL);
        } else {
          network_status.set_reported<node_state_topic_state_t>(
            NODE_STATE_TOPIC_STATE_OFFLINE);
        }
        read_data->network_status = false;
        ctimer_stop(&read_data->timer);
        return;
      }
    }
    ctimer_restart(&read_data->timer);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Shared functions
////////////////////////////////////////////////////////////////////////////////
void network_state_monitor_init()
{
  static zwave_controller_storage_callback_t zwave_controller_storage_callbacks
    = {
      .on_set_node_as_s2_capable
      = zwave_security_validation_set_node_as_s2_capable,
      .on_verify_node_is_s2_capable
      = zwave_security_validation_is_node_s2_capable,
      .on_get_node_granted_keys  = zwave_get_node_granted_keys,
      .on_get_inclusion_protocol = zwave_get_inclusion_protocol,
      .on_zwave_controller_storage_cc_version
      = zwave_node_get_command_class_version,
    };
  zwave_controller_storage_callback_register(
    &zwave_controller_storage_callbacks);
  // Register for ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL attribute
  // update that enables monitoring the sleeping nodes
  attribute_store_register_callback_by_type_and_state(
    network_monitor_wakeup_interval_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL,
    REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type(
    network_monitor_on_wake_up_cc_version_update,
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_VERSION);

  zwave_controller_register_callbacks(&network_monitor_callbacks);
  memset(current_node_list, 0, sizeof(zwave_nodemask_t));

  // Listens to NIF creations, so we can detect node interviews
  attribute_store_register_callback_by_type(network_monitor_on_nif_updated,
                                            ATTRIBUTE_ZWAVE_NIF);

  // Enable the use of UNIDs from other components
  zwave_unid_set_home_id(zwave_network_management_get_home_id());
  new_network_entered_data *data = new new_network_entered_data;
  data->node_id                  = zwave_network_management_get_node_id();
  data->home_id                  = zwave_network_management_get_home_id();
  // At init, if our keys are not in the datastore, we do not want
  // to create a wrong granted_key data, so we ask zwave_network_management()
  data->granted_keys       = zwave_network_management_get_granted_keys();
  data->remove_old_network = false;

  // Execute directly, do not post an event for this, other components
  // initializing right after are depending on us doing the job
  network_monitor_handle_event_new_network(
    static_cast<new_network_entered_data *>(data));

  // Restore the SPAN/MPAN data to S2
  network_monitor_restore_span_table_data();
  network_monitor_restore_mpan_table_data();
}

uint8_t *network_monitor_get_cached_current_node_list()
{
  return current_node_list;
}

////////////////////////////////////////////////////////////////////////////////
// Contiki Process
////////////////////////////////////////////////////////////////////////////////
PROCESS(network_monitor_process, "network_state_monitor_process");

PROCESS_THREAD(network_monitor_process, ev, data)
{
  PROCESS_BEGIN();
  while (1) {
    switch (ev) {
      case PROCESS_EVENT_INIT:
        break;

      case PROCESS_EVENT_EXIT:
        sl_log_info(LOG_TAG, "network_monitor_process is exiting.\n");
        network_monitor_store_span_table_data();
        network_monitor_store_mpan_table_data();
        break;

      case PROCESS_EVENT_EXITED:
        // Do not do anything with this event, just wait to go down.
        break;

      case NEW_NETWORK_EVENT:
        network_monitor_handle_event_new_network(
          static_cast<new_network_entered_data *>(data));
        break;

      case NODE_ID_ASSIGNED_EVENT:
        network_monitor_handle_event_node_id_assigned(
          static_cast<node_id_assigned_event_data *>(data));
        break;

      case NODE_ADDED_EVENT:
        network_monitor_handle_event_node_added(
          static_cast<node_added_event_data *>(data));
        break;

      case NODE_INTERVIEW_INITIATED_EVENT:
        network_monitor_handle_event_node_interview_initiated(
          static_cast<attribute_store_node_t>(
            reinterpret_cast<intptr_t>(data)));
        break;
      case NODE_INTERVIEW_DONE_EVENT:
        network_monitor_handle_event_node_interview_done(
          static_cast<attribute_store_node_t>(
            reinterpret_cast<intptr_t>(data)));
        break;

      case NODE_DELETED_EVENT:
        network_monitor_handle_event_node_deleted(
          (zwave_node_id_t)(intptr_t)data);
        break;

      case NODE_FRAME_TRANSMISSION_FAILED_EVENT:
        network_monitor_handle_event_failed_frame_transmission(
          static_cast<zwave_node_id_t>(reinterpret_cast<intptr_t>(data)));
        break;

      case NODE_FRAME_TRANSMISSION_SUCCESS_EVENT:
        network_monitor_handle_event_success_frame_transmission(
          static_cast<zwave_node_id_t>(reinterpret_cast<intptr_t>(data)));
        break;

      case NODE_FRAME_RX_EVENT:
        network_monitor_handle_event_success_frame_transmission(
          static_cast<zwave_node_id_t>(reinterpret_cast<intptr_t>(data)));
        break;
      default:
        sl_log_warning(LOG_TAG, "Unhandled event %d", ev);
        break;
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
