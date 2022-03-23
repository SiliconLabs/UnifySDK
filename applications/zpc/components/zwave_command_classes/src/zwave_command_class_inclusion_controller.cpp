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
#include "zwave_command_class_inclusion_controller.h"

// Generic includes
#include <assert.h>
#include <map>
#include <cstring>
#include <memory>

// Includes from other components
#include "ZW_classcmd.h"
#include "zwave_command_handler.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_utils.h"
#include "zwave_controller_storage.h"
#include "zwave_network_management.h"
#include "zwave_command_classes_utils.h"
#include "ctimer.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_resolver.h"
#include "zwave_unid.h"
#include "zwave_controller_command_class_indices.h"
#include "sl_log.h"

constexpr char LOG_TAG[] = "zwave_command_class_inclusion_controller";
constexpr clock_time_t ON_NEW_NODE_ADDED_TILL_INITIATE_COMMAND_RECEIVED_TIMEOUT
  = 10000;
constexpr clock_time_t NIF_RESOLUTION_DEFAULT_TIMEOUT = 5000;

typedef enum {
  NOT_STARTED = 0,
  IN_PROGRESS,
  COMPLETED
} security_boostrap_status_t;

typedef struct inclusion_handler_state {
  uint8_t step_id;
  uint8_t status_inclusion_complete;
  zwave_node_info_t node_info;
  zwave_node_id_t node_id;
  zwave_controller_connection_info_t connection_info;
  security_boostrap_status_t security_bootstrap;
  bool nif_received;
  bool on_node_add_received;
  bool initiate_command_received;

  struct ctimer timer_on_node_add_received;
  struct ctimer timer_nif_resolve;

  ~inclusion_handler_state()
  {
    ctimer_stop(&timer_on_node_add_received);
    ctimer_stop(&timer_nif_resolve);
  }
} inclusion_handler_state_t;

static std::map<zwave_node_id_t, std::shared_ptr<inclusion_handler_state_t>>
  node_ids_inclusion_request;

static void zwave_command_class_inclusion_nif_resolve_timer_cc(void *user);
static void
  zwave_command_class_inclusion_timer_on_node_add_received(void *user);

static void zwave_command_class_inclusion_controller_send_s0_intiate(
  zwave_node_id_t node_id)
{
  auto it = node_ids_inclusion_request.find(node_id);
  const ZW_INITIATE_FRAME frame
    = {.cmdClass = COMMAND_CLASS_INCLUSION_CONTROLLER,
       .cmd      = INITIATE,
       .nodeId   = (uint8_t)it->first,
       .stepId   = INITIATE_S0_INCLUSION};
  zwave_command_class_send_report(&(it->second.get()->connection_info),
                                  sizeof(frame),
                                  (const uint8_t *)&frame);
}

static void zwave_command_class_inclusion_controller_send_complete_command(
  uint8_t step_id,
  uint8_t status,
  zwave_controller_connection_info connection_info)
{
  const ZW_COMPLETE_FRAME frame
    = {.cmdClass = COMMAND_CLASS_INCLUSION_CONTROLLER,
       .cmd      = COMPLETE,
       .stepId   = step_id,
       .status   = status};

  zwave_command_class_send_report(&connection_info,
                                  sizeof(frame),
                                  (const uint8_t *)&frame);
}

static void zwave_command_class_inclusion_request_nif(zwave_node_id_t node_id)
{
  // here we are sure that the on node added or initiate commands are received
  auto it = node_ids_inclusion_request.find(node_id);
  if (it != node_ids_inclusion_request.end()) {
    ctimer_stop(&it->second.get()->timer_on_node_add_received);
    ctimer_set(&it->second.get()->timer_nif_resolve,
               NIF_RESOLUTION_DEFAULT_TIMEOUT,
               zwave_command_class_inclusion_nif_resolve_timer_cc,
               it->second.get());
    attribute_store_node_t node_id_node
      = attribute_store_network_helper_get_zwave_node_id_node(node_id);
    attribute_store_node_t endpoint_id_node
      = attribute_store_get_node_child_by_type(node_id_node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               0);
    attribute_store_node_t nif_attribute
      = attribute_store_get_node_child_by_type(endpoint_id_node,
                                               ATTRIBUTE_ZWAVE_NIF,
                                               0);
    if (nif_attribute == ATTRIBUTE_STORE_INVALID_NODE) {
      attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
    }
  }
}

static void zwave_command_class_inclusion_controller_trigger_proxy_inclusion(
  zwave_node_id_t node_id, zwave_node_info_t nif)
{
  auto it = node_ids_inclusion_request.find(node_id);
  if (zwave_controller_storage_is_node_s2_capable(node_id)) {
    if (SL_STATUS_OK
        == zwave_network_management_start_proxy_inclusion(
          it->first,
          it->second.get()->node_info,
          it->second.get()->step_id)) {
      it->second.get()->security_bootstrap = IN_PROGRESS;
    }
  } else if (zwave_node_supports_command_class(COMMAND_CLASS_SECURITY,
                                               it->first,
                                               0)) {
    if (zwave_node_supports_command_class(
          COMMAND_CLASS_SECURITY,
          it->second.get()->connection_info.remote.node_id,
          it->second.get()->connection_info.remote.endpoint_id)) {
      // send intiate command to the inclusion control
      zwave_command_class_inclusion_controller_send_s0_intiate(it->first);
      it->second.get()->security_bootstrap = IN_PROGRESS;
    } else {
      if (SL_STATUS_OK
          == zwave_network_management_start_proxy_inclusion(
            it->first,
            it->second.get()->node_info,
            it->second.get()->step_id)) {
        it->second.get()->security_bootstrap = IN_PROGRESS;
      }
    }
  }
}

static void node_id_resolusion_listener(attribute_store_node_t node)
{
  zwave_node_id_t node_id = 0x00;
  attribute_store_get_reported(node, &node_id, sizeof(zwave_node_id_t));
  auto it = node_ids_inclusion_request.find(node_id);
  if ((it != node_ids_inclusion_request.end())
      && (it->second.get()->security_bootstrap == COMPLETED)) {
    attribute_resolver_clear_resolution_listener(node,
                                                 node_id_resolusion_listener);
    // Send  Inclsuion Controller Complete command to the Inclusion Controller
    zwave_command_class_inclusion_controller_send_complete_command(
      it->second.get()->step_id,
      it->second.get()->status_inclusion_complete,
      it->second.get()->connection_info);
    // Make sure our ref-count doesn't reach zero until we are actually done with the
    // object.
    node_ids_inclusion_request.erase(it);
  }
}

// handler for incoming node information frame
static void zwave_command_class_inclusion_controller_on_nif_update(
  zwave_node_id_t node_id, const zwave_node_info_t *nif)
{
  auto it = node_ids_inclusion_request.find(node_id);
  if ((it != node_ids_inclusion_request.end())
      && !it->second.get()->nif_received) {
    ctimer_stop(&it->second.get()->timer_nif_resolve);
    it->second.get()->nif_received = true;
    it->second.get()->node_info    = *nif;
    if (it->second.get()->nif_received
        && it->second.get()->on_node_add_received) {
      zwave_command_class_inclusion_controller_trigger_proxy_inclusion(
        it->first,
        it->second.get()->node_info);
      attribute_store_node_t node_id_node
        = attribute_store_network_helper_get_zwave_node_id_node(it->first);
      attribute_resolver_pause_node_resolution(node_id_node);
      sl_log_debug(LOG_TAG, "NIF is resolved for NodeID %i", node_id);
    }
  }
}

static void zwave_command_class_inclusion_timer_on_node_add_received(void *user)
{
  // Here new node add received but IC Initiate command does not or vice-versa
  // Therefore, we will assume the node is added non-secure for not compromising the security keys
  // and we trigger the node probing via creating the non-secure NIF.
  inclusion_handler_state_t *read_data
    = static_cast<inclusion_handler_state_t *>(user);
  auto it = node_ids_inclusion_request.find(read_data->node_id);
  // Let's fetch the object before we delete it as removing it from the std::map
  // will most likely cause the ref-count to go to zero otherwise.
  auto actual_object = node_ids_inclusion_request[read_data->node_id];
  node_ids_inclusion_request.erase(it);
  // Create the node id node in attribute store
  unid_t unid;
  zwave_unid_from_node_id(actual_object->node_id, unid);
  attribute_store_node_t endpoint_id_node
    = attribute_store_network_helper_create_endpoint_node(unid, 0);
  attribute_store_node_t nif_attribute
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE_ZWAVE_NIF,
                                             0);
  if (nif_attribute == ATTRIBUTE_STORE_INVALID_NODE) {
    attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  }
  //Here we also need to create the ATTRIBUTE_GRANTED_SECURITY_KEYS
  attribute_store_node_t granted_security_key_node = attribute_store_add_node(
    ATTRIBUTE_GRANTED_SECURITY_KEYS,
    attribute_store_get_node_parent(endpoint_id_node));
  uint8_t granted_key_set = 0x00;
  attribute_store_set_reported(granted_security_key_node,
                               &granted_key_set,
                               sizeof(uint8_t));
}

// A ctimer calback function taht will be called when requested nif is not received
// for a given time inetrval (i.e, 5 seconds). If the requested nif is not received,
// we will consider proxy inclusion failed (i.e., COMPLETE_STEP_FAILED).
static void zwave_command_class_inclusion_nif_resolve_timer_cc(void *user)
{
  inclusion_handler_state_t *read_data
    = static_cast<inclusion_handler_state_t *>(user);
  auto it = node_ids_inclusion_request.find(read_data->node_id);
  if ((it != node_ids_inclusion_request.end())
      && (it->second.get()->security_bootstrap == NOT_STARTED)) {
    // Send  Inclsuion Controller Complete command to the Inclusion Controller
    zwave_command_class_inclusion_controller_send_complete_command(
      it->second.get()->step_id,
      COMPLETE_STEP_FAILED,
      it->second.get()->connection_info);
    // Delete at the end, to make sure our ref-count doesn't reach zero prematurely.
    node_ids_inclusion_request.erase(it);
  }
}

/**
 * @brief A function that wil be called when network management state machine
 *        state is changing. When the state is IDLE, the function will check
 *        if there are any node node_ids_inclusion_request queue which is waiting for security
 *        bootstrapping. If a node presents in the queue the proxy inclusion will be triggred.
 *
 * @param nm_state the network management state
 */
static void
  zwave_command_class_inclusion_controller_on_state_network_management_state_updated(
    zwave_network_management_state_t nm_state)
{
  if (nm_state == NM_IDLE) {
    // If there is a node in the map which security bootstrap does not start but the nif is defined,
    // we trigger security inclusion
    if (!node_ids_inclusion_request.empty()) {
      auto it = node_ids_inclusion_request.begin();
      if ((it->second.get()->security_bootstrap == NOT_STARTED)
          && it->second.get()->nif_received) {
        zwave_command_class_inclusion_controller_trigger_proxy_inclusion(
          it->first,
          it->second.get()->node_info);
      }
    }
  }
}

/**
 * @brief A zwave-controller callback function that will be called when
 *        a new node is joining to the network and assigned a node ID.
 * @param node_id is the node ID of the new node
 * @param included_by_us a flag that describe if the node is added via
 *        ZPC or an Inclusion Controller. (if the node is added via
 *        Inclusion Controller the flag will be FALSE)
 */
static void zwave_command_class_inclusion_controller_on_node_id_assigned(
  zwave_node_id_t node_id, bool included_by_us, zwave_protocol_t protocol)
{
  // We are only interested about nodes included by others
  if (included_by_us) {
    return;
  }
  auto it = node_ids_inclusion_request.find(node_id);
  if (it != node_ids_inclusion_request.end()) {
    it->second.get()->on_node_add_received = true;
    ctimer_stop(&it->second.get()->timer_on_node_add_received);
  } else {
    // fill all possible information for inclusion handler state
    // considering that on_node_assigned zwave_controller callback (i.e, trigger via protocol command)
    // is received before inclusion controller initiate application command
    std::shared_ptr<inclusion_handler_state_t> inclusion_handler_state_info
      = std::make_shared<inclusion_handler_state_t>();
    inclusion_handler_state_info->security_bootstrap        = NOT_STARTED;
    inclusion_handler_state_info->nif_received              = false;
    inclusion_handler_state_info->initiate_command_received = false;
    inclusion_handler_state_info->on_node_add_received      = true;
    inclusion_handler_state_info->node_id                   = node_id;
    ctimer_set(&inclusion_handler_state_info.get()->timer_on_node_add_received,
               ON_NEW_NODE_ADDED_TILL_INITIATE_COMMAND_RECEIVED_TIMEOUT,
               zwave_command_class_inclusion_timer_on_node_add_received,
               inclusion_handler_state_info.get());

    node_ids_inclusion_request.try_emplace(node_id,
                                           inclusion_handler_state_info);
  }
  // Create the node id node in attribute store
  unid_t unid;
  zwave_unid_from_node_id(node_id, unid);
  attribute_store_network_helper_create_endpoint_node(unid, 0);
  it = node_ids_inclusion_request.find(node_id);
  if (it->second.get()->initiate_command_received
      && it->second.get()->on_node_add_received) {
    zwave_command_class_inclusion_request_nif(it->first);
  }
}

// A Z-Wave controller callback function that will be called when a security inclusion is completed.
static void zwave_command_class_inclusion_controller_on_node_added_completed(
  sl_status_t status,
  const zwave_node_info_t *nif,
  zwave_node_id_t node_id,
  const zwave_dsk_t dsk,
  zwave_keyset_t granted_keys,
  zwave_kex_fail_type_t kex_fail_type,
  zwave_protocol_t inclusion_protocol)
{
  auto it = node_ids_inclusion_request.find(node_id);
  if ((it != node_ids_inclusion_request.end())
      && (it->second.get()->security_bootstrap == IN_PROGRESS)) {
    it->second.get()->security_bootstrap = COMPLETED;
    attribute_store_node_t node_id_node
      = attribute_store_network_helper_get_zwave_node_id_node(node_id);
    attribute_resolver_resume_node_resolution(node_id_node);
    attribute_resolver_set_resolution_listener(node_id_node,
                                               node_id_resolusion_listener);
    // Map kex_fail_type to Inclusion Controller Complete Command status
    switch (kex_fail_type) {
      case ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE:
        it->second.get()->status_inclusion_complete = COMPLETE_STEP_OK;
        break;
      case ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEX_KEY:
      case ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_DECRYPT:
      case ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_AUTH:
      case ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEY_GET:
      case ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEY_VERIFY:
      case ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEY_REPORT:
      case ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_UNKNOWN:
        it->second.get()->status_inclusion_complete = COMPLETE_STEP_FAILED;
        break;
      case ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEX_SCHEME:
      case ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEX_CURVES:
        it->second.get()->status_inclusion_complete
          = COMPLETE_STEP_NOT_SUPPORTED;
        break;
      case ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_CANCEL:
        it->second.get()->status_inclusion_complete
          = COMPLETE_STEP_USER_REJECTED;
        break;
      default:
        it->second.get()->status_inclusion_complete = COMPLETE_STEP_FAILED;
        break;
    }
  }
}

/**
 * @brief Verifies if an Inclusion Controller frame should be accepted or not.
 *
 * Refer to CC:0074.01.01.11.001 for Initiate and CC:0074.01.02.11.002 for Complete
 *
 * @param node_id         The NodeID of the remote controller sending us a command.
 *
 * @returns true if the command should be accepted, false otherwise.
 */
static bool
  is_command_security_valid(const zwave_controller_connection_info_t *info)
{
  // Get the keys of the other controller
  zwave_keyset_t remote_node_keys;
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(
      info->remote.node_id);
  attribute_store_node_t granted_keys_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                             0);
  if (SL_STATUS_OK
      != attribute_store_get_reported(granted_keys_node,
                                      &remote_node_keys,
                                      sizeof(zwave_keyset_t))) {
    // If we cannot read the granted keys, it can be that we do not know for sure
    // or perhaps the node is not even in our network.
    // Reject the frame in such a case.
    sl_log_warning(LOG_TAG,
                   "Cannot find granted keys for NodeID %d. "
                   "Rejecting Inclusion Controller Command.",
                   info->remote.node_id);
    return false;
  }

  // Get both parties scheme.
  zwave_controller_encapsulation_scheme_t zpc_scheme
    = zpc_highest_security_class();
  zwave_controller_encapsulation_scheme_t remote_scheme
    = zwave_controller_get_highest_encapsulation(remote_node_keys);

  // Now compare, we expect the connection info to at highest common scheme
  zwave_controller_encapsulation_scheme_t common_scheme = zpc_scheme;
  if (zwave_controller_encapsulation_scheme_greater_equal(zpc_scheme,
                                                          remote_scheme)) {
    common_scheme = remote_scheme;
  }

  if (zwave_controller_encapsulation_scheme_greater_equal(info->encapsulation,
                                                          common_scheme)) {
    return true;
  } else {
    sl_log_info(LOG_TAG,
                "Rejecting Inclusion Controller Command due to insufficient "
                "security level.");
    return false;
  }
}

/////////////////////////////////////////////////////////////////////////
//Command Handler
/////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_inclusion_controller_support_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 4) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_INCLUSION_CONTROLLER) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (false == is_command_security_valid(connection_info)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_INDEX] == INITIATE) {
    const ZW_INITIATE_FRAME *initiate_frame
      = reinterpret_cast<const ZW_INITIATE_FRAME *>(frame_data);
    auto it = node_ids_inclusion_request.find(initiate_frame->nodeId);
    if (it == node_ids_inclusion_request.end()) {
      // fill all possible information for inclusion handler state
      // considering that inclusion controller initiate application command is received
      // before on_node_assigned zwave_controller callback (i.e, trigger via protocol command)
      std::shared_ptr<inclusion_handler_state_t> inclusion_handler_state_info
        = std::make_shared<inclusion_handler_state_t>();
      inclusion_handler_state_info->connection_info    = *connection_info;
      inclusion_handler_state_info->step_id            = initiate_frame->stepId;
      inclusion_handler_state_info->security_bootstrap = NOT_STARTED;
      inclusion_handler_state_info->nif_received       = false;
      inclusion_handler_state_info->initiate_command_received = true;
      inclusion_handler_state_info->on_node_add_received      = false;
      inclusion_handler_state_info->node_id = initiate_frame->nodeId;
      ctimer_set(
        &inclusion_handler_state_info.get()->timer_on_node_add_received,
        ON_NEW_NODE_ADDED_TILL_INITIATE_COMMAND_RECEIVED_TIMEOUT,
        zwave_command_class_inclusion_timer_on_node_add_received,
        inclusion_handler_state_info.get());

      node_ids_inclusion_request.try_emplace(initiate_frame->nodeId,
                                             inclusion_handler_state_info);

    } else {
      if (it->second.get()->on_node_add_received) {
        ctimer_stop(&it->second.get()->timer_on_node_add_received);
        it->second.get()->initiate_command_received = true;
        it->second.get()->step_id                   = initiate_frame->stepId;
        it->second.get()->connection_info           = *connection_info;
      }
    }
    // Here we are sure that the the joining node is in the node_ids_inclusion_request map list,
    // Therefore, we check if both the IC initiate application and on new node added protocol commands
    // are received. If they are received we create a NIF attribute and issue request node info.
    it = node_ids_inclusion_request.find(initiate_frame->nodeId);
    if (it->second.get()->initiate_command_received
        && it->second.get()->on_node_add_received) {
      zwave_command_class_inclusion_request_nif(it->first);
      return SL_STATUS_OK;
    }
    if (initiate_frame->stepId == INITIATE_PROXY_INCLUSION_REPLACE) {
      // If it is a proxy replace, first we delete the node attribute and create it again
      attribute_store_node_t node_id_node_to_replace
        = attribute_store_network_helper_get_zwave_node_id_node(
          initiate_frame->nodeId);
      attribute_store_delete_node(node_id_node_to_replace);
      unid_t unid;
      zwave_unid_from_node_id(initiate_frame->nodeId, unid);
      attribute_store_node_t endpoint_node
        = attribute_store_network_helper_create_endpoint_node(unid, 0);
      attribute_store_node_t node_id_node
        = attribute_store_get_node_parent(endpoint_node);
      attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
      // During replace fail, on_node_assigned zwave_controller callback (i.e, trigger via protocol command)
      // will not be received. Therefore, we trigger the nif interview when we only receive INITIATE command
      it->second.get()->on_node_add_received = true;
      zwave_command_class_inclusion_request_nif(it->first);
    }
  } else if (frame_data[COMMAND_INDEX] == COMPLETED) {
    const ZW_COMPLETE_FRAME *complete_frame
      = reinterpret_cast<const ZW_COMPLETE_FRAME *>(frame_data);
    for (auto it = node_ids_inclusion_request.begin();
         it != node_ids_inclusion_request.end();
         ++it) {
      if (it->second.get()->security_bootstrap == IN_PROGRESS) {
        it->second.get()->security_bootstrap        = COMPLETED;
        it->second.get()->status_inclusion_complete = complete_frame->status;
        attribute_store_node_t node_id_node
          = attribute_store_network_helper_get_zwave_node_id_node(it->first);
        attribute_resolver_resume_node_resolution(node_id_node);
        attribute_resolver_set_resolution_listener(node_id_node,
                                                   node_id_resolusion_listener);
        attribute_store_node_t granted_security_key_node
          = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                     node_id_node);
        uint8_t granted_key_set = 0x00;
        if (complete_frame->status == COMPLETE_STEP_OK) {
          granted_key_set
            = ZWAVE_CONTROLLER_S0_KEY;  // Here we are sure that the node is added with security 0
        }
        attribute_store_set_reported(granted_security_key_node,
                                     &granted_key_set,
                                     sizeof(uint8_t));
        break;
      }
    }
  }

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_inclusion_controller_init()
{
  static zwave_controller_callbacks_t cb = {
    .on_state_updated
    = zwave_command_class_inclusion_controller_on_state_network_management_state_updated,
    .on_node_id_assigned
    = zwave_command_class_inclusion_controller_on_node_id_assigned,
    .on_node_added
    = zwave_command_class_inclusion_controller_on_node_added_completed,
    .on_node_information
    = zwave_command_class_inclusion_controller_on_nif_update,
  };
  zwave_controller_register_callbacks(&cb);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  // Since the command handler receive and send inclusion controller commands,
  // we register the same handler for both control and support.
  handler.control_handler
    = zwave_command_class_inclusion_controller_support_handler;
  handler.support_handler
    = zwave_command_class_inclusion_controller_support_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.command_class      = COMMAND_CLASS_INCLUSION_CONTROLLER;
  handler.command_class_name = "Inclusion Controller";
  handler.version            = INCLUSION_CONTROLLER_VERSION;
  /// TODO: UIC-1102 Wait for the 2021D spec release to be in force for cert and set that to false again.
  handler.manual_security_validation = true;

  zwave_command_handler_register_handler(handler);
  return SL_STATUS_OK;
}
