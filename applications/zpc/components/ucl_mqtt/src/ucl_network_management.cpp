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
#include "process.h"
#include "sl_log.h"
#include "sl_status.h"
#include "ucl_network_management.h"
#include "uic_mqtt.h"
#include "zpc_converters.h"
#include "zwave_controller_callbacks.h"
#include "zwave_network_management.h"
#include "zwave_unid.h"
#include "sys/etimer.h"
#include "ucl_nm_neighbor_discovery.h"

#include <boost/iostreams/stream.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cstddef>
#include <map>
#include <sstream>
#include <deque>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
// Defines and types
////////////////////////////////////////////////////////////////////////////////
#define LOG_TAG "ucl_network_management"

namespace bpt = boost::property_tree;

/**
 * @brief @ref Contiki Event definitions for the Network Monitor Process.
 */
typedef enum {
  /// The ZPC entered a new network
  NEW_NETWORK_EVENT,
  /// A new node was added to the network
  NODE_ADDED_EVENT,
  /// Node interview done
  NODE_DELETED_EVENT,
  ///
  MQTT_CB_EVENT,
  /// Update the UclNetworkManagement State (on MQTT)
  UPDATE_STATE_EVENT,
  /// Trigger removing offline node
  UCL_NM_REMOVE_OFFLINE_EVENT,
  /// Trigger node neighbor request update
  UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_EVENT
} network_monitor_events_t;

/**
 * @brief Possible State values for the ZPC in the NetworkManagement topic.
 */
typedef enum {
  /// The network management state machine is idle and ready to carry operations
  UCL_NM_TOPIC_IDLE,
  /// Include a new node into the network
  UCL_NM_TOPIC_ADD_NODE,
  /// Remove any or a specific node from the network
  UCL_NM_TOPIC_REMOVE_NODE,
  /// Remove a non-responsive or non-reachable node from the network topology
  UCL_NM_TOPIC_REMOVE_FAILED_NODE,
  /// ZPC is joining a new network
  UCL_NM_TOPIC_JOIN_NETWORK,
  /// ZPC is leaving the current network
  UCL_NM_TOPIC_LEAVE_NETWORK,
  /// ZPC is interviewing a node
  UCL_NM_TOPIC_NODE_INTERVIEW,
  /// The Protocol Controller is carrying out some network repair functions,
  /// such as providing routes for nodes or
  /// remove bindings towards nodes that have left the network
  UCL_NM_TOPIC_NETWORK_REPAIR,
  /// The Protocol Controller is carrying out some network maintenance,
  /// which can for example be distributing
  /// the network topology or state to all other controller nodes in the network.
  UCL_NM_TOPIC_NETWORK_MAINTENANCE,
  /// The Protocol Controller is resetting to default
  UCL_NM_TOPIC_RESET,
  /// The Protocol Controller goes offline and will not be reachable at all.
  /// It can be for example when it needs to restart after a firmware update
  UCL_NM_TOPIC_TEMPORARILY_OFFLINE,
  /// The Protocol Controller is looking for nodes to join the network,
  /// and will provide a list of candidates. No nodes are added yet
  UCL_NM_TOPIC_SCAN_MODE,
  /// This state shall always be last
  UCL_NM_TOPIC_LAST,
} ucl_network_management_state_t;

/**
 * @brief Struct used for sending event data with event \ref NEW_NETWORK_EVENT.
 */
struct new_network_entered_data {
  zwave_home_id_t home_id;              ///< Home ID
  zwave_node_id_t node_id;              ///< Node ID
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
};

/**
 * @brief Object to hold data for \ref state_topic_update
 */
class UclNetworkManagementStateData
{
  public:
  const ucl_network_management_state_t state;
  const std::vector<std::pair<std::string, std::string>> state_parameters;
  const std::vector<std::string> requested_state_parameters;

  /**
   * @brief Construct a new Uic State Topic Update Data object
   *
   * @param state
   * @param state_parameters
   * @param requested_state_parameters
   */
  UclNetworkManagementStateData(
    ucl_network_management_state_t state,
    const std::vector<std::pair<std::string, std::string>> &state_parameters,
    const std::vector<std::string> &requested_state_parameters) :
    state(state),
    state_parameters(state_parameters),
    requested_state_parameters(requested_state_parameters)
  {}

  /**
   * @brief Construct a new Uic State Topic Update Data object
   *
   * @param state
   * @param state_parameters
   */
  UclNetworkManagementStateData(
    ucl_network_management_state_t state,
    const std::vector<std::pair<std::string, std::string>> &state_parameters) :
    UclNetworkManagementStateData(
      state, state_parameters, std::vector<std::string>())
  {}

  /**
   * @brief Construct a new Uic State Topic Update Data object
   *
   * @param state
   */
  explicit UclNetworkManagementStateData(ucl_network_management_state_t state,
                                         const std::vector<std::string> &) :
    UclNetworkManagementStateData(
      state,
      std::vector<std::pair<std::string, std::string>>(),
      requested_state_parameters)

  {}

  /**
   * @brief Construct a new Uic State Topic Update Data object
   *
   * @param state
   */
  explicit UclNetworkManagementStateData(ucl_network_management_state_t state) :
    UclNetworkManagementStateData(
      state,
      std::vector<std::pair<std::string, std::string>>(),
      std::vector<std::string>())
  {}
};
// list of offline nodes that will be removed from the network.
static std::deque<zwave_node_id_t> offline_node_list;
static struct etimer offline_node_timer;
////////////////////////////////////////////////////////////////////////////////
// Local Function Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Trigger the network management state based on received ZPC NetworkManagement
 * Write topic(ucl/by-unid/<zpc-unid>/ProtocolController/NetworkManagement/Write) message.
 *
 * @param message A NetworkManagement message that will be received from MQTT client
 *
 * @returns SL_STATUS_OK if it went well
 * @returns SL_STATUS_FAIL if it did not
 */
static sl_status_t write_topic_received(const std::string &message);

/**
 * @brief Subscribe to ZPC network management state topics
 *
 * This function will subscribe to the following topic:
 * ucl/by-unid/&lt;zpc-unid&gt;/ProtocolController/NetworkManagement/Write
 *
 * @param unid     Unique node identifier of ZPC
 * @param callBack A callback function,that will be called
 *                 when a NetworkManagement message is received from the broker.
 *
 * @returns SL_STATUS_OK if it went well
 * @returns SL_STATUS_FAIL if it did not
 *
 */
static sl_status_t
  write_topic_subscribe(const unid_t unid,
                        void (*callBack)(const char *topic,
                                         const char *message,
                                         const size_t message_length));

/**
 * @brief Unsubscribe to ZPC network management state topics
 *
 * This function will unsubscribe to the following topic:
 * -> ucl/by-unid/<zpc-unid>/ProtocolController/NetworkManagement/Write
 *
 * @param unid     Unique node identifier of ZPC
 * @param callBack A callback function,that will be called
 *                 when a NetworkManagement message is received from the broker.
 *
 * @returns SL_STATUS_OK if it went well
 * @returns SL_STATUS_FAIL if it did not
 *
 */
static sl_status_t
  write_topic_unsubscribe(const unid_t unid,
                          void (*callBack)(const char *topic,
                                           const char *message,
                                           const size_t message_length));

/**
 * @brief publish a new ZPC network management state
 *
 * This function will publish a retained
 *    ucl/by-unid/&lt;unid&gt;/ProtocolController/NetworkManagement state message
 *
 * @param unid                        Unique node identifier
 * @param uic_state_topic_update_data State data to publish
 *
 * @returns SL_STATUS_OK if it went well
 * @returns SL_STATUS_FAIL if it did not
 *
 */
static sl_status_t state_topic_update(
  const unid_t unid,
  const UclNetworkManagementStateData &uic_state_topic_update_data);

// Network Management Callback functions

static void ucl_network_management_on_state_updated(
  zwave_network_management_state_t nm_state);
static void ucl_network_management_on_keys_report(bool csa,
                                                  zwave_keyset_t keys);
static void ucl_network_management_on_dsk_report(uint8_t input_length,
                                                 zwave_dsk_t dsk,
                                                 zwave_keyset_t keys);
static void ucl_network_management_on_new_network_entered(
  zwave_home_id_t __home_id,
  zwave_node_id_t __node_id,
  zwave_keyset_t __granted_keys,
  zwave_kex_fail_type_t kex_fail_type);

static void
  ucl_network_management_on_node_added(sl_status_t status,
                                       const zwave_node_info_t *nif,
                                       zwave_node_id_t node_id,
                                       zwave_dsk_t dsk,
                                       zwave_keyset_t granted_keys,
                                       zwave_kex_fail_type_t kex_fail_type,
                                       zwave_protocol_t inclusion_protocol);

// Contiki event helper handler functions

static void ucl_network_management_init();
static void ucl_network_management_exit();
static void
  ucl_network_management_new_network_ev(new_network_entered_data *event_data);

////////////////////////////////////////////////////////////////////////////////
// Local Variables and Consts
////////////////////////////////////////////////////////////////////////////////

// Use boost::property_tree::detail::less_nocase as comperator
// to make map keys case insensitive
static const std::map<std::string,
                      ucl_network_management_state_t,
                      bpt::detail::less_nocase<std::string>>
  ucl_network_management_state_map = {
    {"idle", UCL_NM_TOPIC_IDLE},
    {"add node", UCL_NM_TOPIC_ADD_NODE},
    {"remove node", UCL_NM_TOPIC_REMOVE_NODE},
    {"join network", UCL_NM_TOPIC_JOIN_NETWORK},
    {"leave network", UCL_NM_TOPIC_LEAVE_NETWORK},
    {"network repair", UCL_NM_TOPIC_NETWORK_REPAIR},
    {"network maintenance", UCL_NM_TOPIC_NETWORK_MAINTENANCE},
    {"reset", UCL_NM_TOPIC_RESET},
    {"temporarily offline", UCL_NM_TOPIC_TEMPORARILY_OFFLINE},
    {"scan mode", UCL_NM_TOPIC_SCAN_MODE},
};

static const zwave_controller_callbacks_t ucl_network_management_callbacks = {
  .on_state_updated       = ucl_network_management_on_state_updated,
  .on_node_added          = ucl_network_management_on_node_added,
  .on_new_network_entered = ucl_network_management_on_new_network_entered,
  .on_keys_report         = ucl_network_management_on_keys_report,
  .on_dsk_report          = ucl_network_management_on_dsk_report,
};

static bool allow_multiple_inclusions = false;
static unid_t zpc_unid;

////////////////////////////////////////////////////////////////////////////////
// Local Function Implementations
////////////////////////////////////////////////////////////////////////////////

// Network Management Callbacks

static void ucl_network_management_on_state_updated(
  zwave_network_management_state_t nm_state)
{
  static ucl_network_management_state_t ucl_nm_state;
  switch (nm_state) {
    case NM_IDLE:
      ucl_nm_state = UCL_NM_TOPIC_IDLE;
      break;
    case NM_WAITING_FOR_ADD:
      ucl_nm_state = UCL_NM_TOPIC_ADD_NODE;
      break;
    case NM_NODE_FOUND:
    case NM_WAIT_FOR_PROTOCOL:
    case NM_WAIT_FOR_SECURE_ADD:
    case NM_PREPARE_SUC_INCLISION:
    case NM_WAIT_FOR_SUC_INCLUSION:
      return;  // ignore above states
    case NM_PROXY_INCLUSION_WAIT_NIF:
      ucl_nm_state = UCL_NM_TOPIC_NODE_INTERVIEW;
      break;
    case NM_SET_DEFAULT:
      ucl_nm_state = UCL_NM_TOPIC_RESET;
      break;
    case NM_LEARN_MODE:
    case NM_LEARN_MODE_STARTED:
    case NM_WAIT_FOR_SECURE_LEARN:
    case NM_WAIT_FOR_PROBE_BY_SIS:
      ucl_nm_state = UCL_NM_TOPIC_JOIN_NETWORK;
      break;
    case NM_WAITING_FOR_NODE_REMOVAL:
      ucl_nm_state = UCL_NM_TOPIC_REMOVE_NODE;
      break;
    case NM_WAITING_FOR_FAILED_NODE_REMOVAL:
    case NM_REPLACE_FAILED_REQ:
    case NM_SEND_NOP:
    case NM_WAIT_FOR_TX_TO_SELF_DESTRUCT:
    case NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL:
    case NM_FAILED_NODE_REMOVE:
      ucl_nm_state = UCL_NM_TOPIC_REMOVE_NODE;
      break;
    default:
      sl_log_warning(LOG_TAG, "unhandled network management state");
      break;
  }
  process_post(&ucl_network_management_process,
               UPDATE_STATE_EVENT,
               new UclNetworkManagementStateData(ucl_nm_state));
}

/**
 * @brief Automatically accept all keys
 *
 * @param csa
 * @param keys
 */
static void ucl_network_management_on_keys_report(bool csa, zwave_keyset_t keys)
{
  // We automatically accept all keys
  sl_status_t status = zwave_network_management_keys_set(true, csa, keys);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "zwave_network_management_keys_set failed: %d",
                 status);
  }
}

static void ucl_network_management_on_new_network_entered(
  zwave_home_id_t home_id,
  zwave_node_id_t node_id,
  zwave_keyset_t granted_keys,
  zwave_kex_fail_type_t kex_fail_type)
{
  new_network_entered_data *data = new new_network_entered_data;
  data->node_id                  = node_id;
  data->home_id                  = home_id;
  data->granted_keys             = granted_keys;
  data->remove_old_network       = true;
  process_post(&ucl_network_management_process, NEW_NETWORK_EVENT, data);
}

/**
 * @brief Handle on_dsk_report callback.
 *
 * Signals \ref zpc_state_topic with `add node` with dsk and
 *
 * This function requests a user to insert the missing part of the DSK if the inclusion
 * needs authentication. If the inclusion is unauthenticated, ZPC does not
 * notify the user about the received DSK, rather the ZPC will accept the received DSK.
 *
 * @param input_length
 * @param dsk
 * @param keys
 */
static void ucl_network_management_on_dsk_report(uint8_t input_length,
                                                 zwave_dsk_t dsk,
                                                 zwave_keyset_t keys)
{
  if ((keys & ZWAVE_CONTROLLER_S2_ACCESS_KEY)
      || (keys & ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY)) {
    char dsk_str[DSK_STR_LEN];
    // Input validation of 'input_length'
    if (input_length % 2 != 0) {
      sl_log_error(LOG_TAG,
                   "number of leading bytes to 'x' out in dsk is 'odd' (%u), "
                   "which isn't supported. Must be an even number",
                   input_length);
      return;
    }
    // More input validation of 'input_length'
    if (input_length > ZWAVE_DSK_LENGTH) {
      sl_log_error(
        LOG_TAG,
        "number of leading bytes to 'x' out is larger than the number "
        "of bytes in the dsk, it is: %u",
        input_length);
      return;
    }
    if (zpc_converters_dsk_to_str(dsk, dsk_str, sizeof(dsk_str))
        != SL_STATUS_OK) {
      sl_log_error(LOG_TAG, "Failed to convert DSK to string");
      return;
    }
    // Blank out 'input_length' bytes with 'x'
    for (unsigned int i = 0; i < input_length / 2; i++) {
      // Get the position of 'I' in the following sequence: Ixxxx-Ixxxx-Ixxxx-...
      unsigned int index = i * 5 + i;
      for (unsigned int j = 0; j < 5; j++) {
        dsk_str[index + j] = 'x';
      }
    }
    process_post(&ucl_network_management_process,
                 UPDATE_STATE_EVENT,
                 new UclNetworkManagementStateData(
                   UCL_NM_TOPIC_ADD_NODE,
                   {{"ProvisioningMode", "ZWaveDSK"},
                    {"SecurityCode", std::string(dsk_str)}},
                   {"SecurityCode", "UserAccept", "AllowMultipleInclusions"}));
  } else {
    // If the highest requested key is unauthenticated, we accept the received DSK directly
    // since the inclusion does not need authentication.
    zwave_network_management_dsk_set(dsk);
  }
}

static void
  ucl_network_management_on_node_added(sl_status_t status,
                                       const zwave_node_info_t *nif,
                                       zwave_node_id_t node_id,
                                       zwave_dsk_t dsk,
                                       zwave_keyset_t granted_keys,
                                       zwave_kex_fail_type_t kex_fail_type,
                                       zwave_protocol_t inclusion_protocol)
{
  process_post(&ucl_network_management_process,
               UPDATE_STATE_EVENT,
               new UclNetworkManagementStateData(UCL_NM_TOPIC_IDLE));
}

// MQTT handler function

/**
 * Helper function for getting the ucl/by-unid/<unid>/ProtocolController
 * string.
 */
static std::string get_topic(const unid_t unid)
{
  return "ucl/by-unid/" + std::string(unid) + "/ProtocolController";
}

static sl_status_t write_topic_received(const std::string &message)
{
  std::stringstream ss;
  ss << message;
  bpt::iptree root_tree;
  try {
    bpt::json_parser::read_json(ss, root_tree);
  } catch (const bpt::json_parser_error &e) {
    sl_log_error(LOG_TAG,
                 "Failed parsing json message: '%s', error: %s",
                 message.c_str(),
                 e.what());
    return SL_STATUS_FAIL;
  }
  try {
    std::string mesg = root_tree.get<std::string>("State");
    sl_log_debug(LOG_TAG, "State: '%s' received", mesg.c_str());
    auto state = ucl_network_management_state_map.find(mesg);
    if (state == ucl_network_management_state_map.end()) {
      sl_log_error(LOG_TAG, "Invalid state: %s", mesg.c_str());
      return SL_STATUS_FAIL;
    }

    switch (state->second) {
      case UCL_NM_TOPIC_IDLE:
        allow_multiple_inclusions = false;
        zwave_network_management_abort();
        break;
      case UCL_NM_TOPIC_ADD_NODE: {
        if (auto state_parameters = root_tree.get_child_optional(
              "StateParameters")) {  // User have supplied StateParameters
          allow_multiple_inclusions
            = state_parameters.get()
                .get_optional<bool>("AllowMultipleInclusions")
                .get_value_or(allow_multiple_inclusions);
          if (boost::optional<bool> user_accept
              = state_parameters.get().get_optional<bool>(
                "UserAccept")) {      // User have supplied UserAccepted
            if (user_accept.get()) {  // UserAccepted is true
              sl_log_debug(LOG_TAG, "User accepted node add");
              if (auto dsk_str
                  = state_parameters.get().get_optional<std::string>(
                    "SecurityCode")) {  // User have supplied SecurityCode
                zwave_dsk_t zwave_dsk;
                if (SL_STATUS_OK
                    == zpc_converters_dsk_str_to_internal(dsk_str.get().c_str(),
                                                          zwave_dsk)) {
                  zwave_network_management_dsk_set(zwave_dsk);
                } else {  // Failed to parse dsk
                  sl_log_warning(LOG_TAG,
                                 "Failed to parse dsk: '%s'",
                                 dsk_str.get().c_str());
                }
              } else {  // UserAccepted is true, but SecurityCode is not supplied
                sl_log_warning(
                  LOG_TAG,
                  "\"State\": \"add node\" is missing \"StateParameters\" "
                  "\"SecurityCode\", discarting message: %s",
                  message.c_str());
              }
            } else {  // UserAccepted is false
              sl_log_debug(LOG_TAG, "User rejected node add");
              zwave_network_management_abort();
            }
          } else {  // UserAccept have not been supplied
            zwave_network_management_add_node();
          }
        } else {  // User have not supplied StateParameters, assuming add node
          zwave_network_management_add_node();
        }
      } break;
      case UCL_NM_TOPIC_REMOVE_NODE:
        zwave_network_management_remove_node();

        break;
      case UCL_NM_TOPIC_JOIN_NETWORK:
        break;
      case UCL_NM_TOPIC_LEAVE_NETWORK:
        break;
      case UCL_NM_TOPIC_NETWORK_REPAIR:
        break;
      case UCL_NM_TOPIC_NETWORK_MAINTENANCE:
        break;
      case UCL_NM_TOPIC_RESET:
        zwave_controller_reset();
        break;
      case UCL_NM_TOPIC_TEMPORARILY_OFFLINE:
        break;
      case UCL_NM_TOPIC_SCAN_MODE:
        break;
      case UCL_NM_TOPIC_LAST:
        break;
      default:
        sl_log_warning(LOG_TAG, "Unhandled UCL Network Management Topic");
        break;
    }
  } catch (const bpt::ptree_error &e) {
    sl_log_error(LOG_TAG,
                 "JSON payload invalid %s, Error: %s",
                 message.c_str(),
                 e.what());
  }
  return SL_STATUS_OK;
}

static sl_status_t
  write_topic_subscribe(const unid_t unid,
                        void (*callBack)(const char *topic,
                                         const char *message,
                                         const size_t message_length))
{
  sl_log_debug(
    LOG_TAG,
    ("Subscribing to: " + get_topic(unid) + "/NetworkManagement/Write")
      .c_str());
  std::string wat(get_topic(unid) + "/NetworkManagement/Write");
  uic_mqtt_subscribe(wat.data(), callBack);
  return SL_STATUS_OK;
}

static sl_status_t
  write_topic_unsubscribe(const unid_t unid,
                          void (*callBack)(const char *topic,
                                           const char *message,
                                           const size_t message_length))
{
  uic_mqtt_unsubscribe((get_topic(unid) + "/NetworkManagement/Write").data(),
                       callBack);
  return SL_STATUS_OK;
}

static sl_status_t state_topic_update(
  const unid_t unid,
  const UclNetworkManagementStateData &uic_state_topic_update_data)
{
  // According to spec, when AllowMultipleInclusions is true, it shall remain
  // in add node when first add node is finished. It shall remain in this state,
  // until a "State": "idle" is received on the MQTT interface.
  if (allow_multiple_inclusions
      && uic_state_topic_update_data.state == UCL_NM_TOPIC_IDLE) {
    sl_log_debug(LOG_TAG,
                 "allow_multiple_inclusions is true, and state is 'idle', "
                 "triggering add_node again");
    zwave_network_management_add_node();
    return SL_STATUS_OK;
  }
  // find key in map where value match uic_state (reverse lookup)
  for (auto it = ucl_network_management_state_map.begin();
       it != ucl_network_management_state_map.end();
       ++it) {
    if (it->second == uic_state_topic_update_data.state) {
      bpt::iptree root;
      root.put("State", it->first);

      // StateParameters
      bpt::iptree state_parameters;
      for (auto sp_it = uic_state_topic_update_data.state_parameters.begin();
           sp_it != uic_state_topic_update_data.state_parameters.end();
           ++sp_it) {
        state_parameters.put(sp_it->first, sp_it->second);
      }
      if (!state_parameters.empty()) {
        root.add_child("StateParameters", state_parameters);
      }

      // RequestedStateParamters
      bpt::iptree requested_state_parameters;
      for (auto rsp_it
           = uic_state_topic_update_data.requested_state_parameters.begin();
           rsp_it
           != uic_state_topic_update_data.requested_state_parameters.end();
           ++rsp_it) {
        bpt::iptree child;
        child.put("", *rsp_it);
        requested_state_parameters.push_back(std::make_pair("", child));
      }
      if (!requested_state_parameters.empty()) {
        root.add_child("RequestedStateParameters", requested_state_parameters);
      }
      std::stringstream ss;
      bpt::json_parser::write_json(ss, root);
      std::string msg = ss.str();
      uic_mqtt_publish((get_topic(unid) + "/NetworkManagement").data(),
                       msg.data(),
                       msg.size(),
                       true);
      return SL_STATUS_OK;
    }
  }
  sl_log_error(LOG_TAG,
               "Invalid topic state: %d",
               uic_state_topic_update_data.state);
  return SL_STATUS_FAIL;
}

static void ucl_network_management_mqtt_callback(const char *topic,
                                                 const char *message,
                                                 const size_t message_length)
{
  sl_log_debug(LOG_TAG,
               "ZPC NetworkManagement received Topic: %s state: %s\n",
               topic,
               message);
  if (message == nullptr || message_length == 0) {
    return;
  }
  process_post(&ucl_network_management_process,
               MQTT_CB_EVENT,
               new std::string(message));
}

////////////////////////////////////////////////////////////////////////////////
// Contiki Event Handlers
////////////////////////////////////////////////////////////////////////////////
static void ucl_network_management_init()
{
  assert("ucl_network_management_state_map doesn't contain all elements from "
         "ucl_network_management_state_t"
         && ucl_network_management_state_map.size() != UCL_NM_TOPIC_LAST - 1);
  zwave_controller_register_callbacks(&ucl_network_management_callbacks);
  new_network_entered_data *data = new new_network_entered_data;
  data->node_id                  = zwave_network_management_get_node_id();
  data->home_id                  = zwave_network_management_get_home_id();
  // At init, if our keys are not in the datastore, we do not want
  // to create a wrong granted_key data, so we ask zwave_network_management()
  data->granted_keys       = zwave_network_management_get_granted_keys();
  data->remove_old_network = false;
  ucl_network_management_new_network_ev(data);
  ucl_network_management_on_state_updated(zwave_network_management_get_state());
  /// intialize ucl network management node neighbor discovery submodule
  ucl_nm_neighbor_discovery_init();
}

static void ucl_network_management_exit()
{
  write_topic_unsubscribe(zpc_unid, ucl_network_management_mqtt_callback);
  uic_mqtt_unretain((get_topic(zpc_unid) + "/NetworkManagement").c_str());
}

static void
  ucl_network_management_new_network_ev(new_network_entered_data *event_data)
{
  // If we just started, do not try to clean up the old network
  if (event_data->remove_old_network) {
    // Stop MQTT subscription to the previous network
    write_topic_unsubscribe(zpc_unid, ucl_network_management_mqtt_callback);
  }
  // Configure our new HomeID
  zwave_unid_set_home_id(event_data->home_id);
  //Subscribe to our new network management topic
  zwave_unid_from_node_id(event_data->node_id, zpc_unid);
  write_topic_subscribe(zpc_unid, ucl_network_management_mqtt_callback);
  delete event_data;
}

static void ucl_network_management_remove_offline_node_manager()
{
  if (zwave_network_management_get_state() == NM_IDLE) {
    zwave_network_management_remove_failed(offline_node_list.front());
    offline_node_list.pop_front();
  } else {
    etimer_restart(&offline_node_timer);
  }
}

static void
  ucl_network_management_trigger_remove_offline_node(zwave_node_id_t node_id)
{
  if (offline_node_list.empty()) {
    etimer_set(&offline_node_timer, UCL_NM_REMOVE_FAILED_NODE_TIMEOUT_DEFAULT);
  }
  std::deque<zwave_node_id_t>::iterator it
    = find(offline_node_list.begin(), offline_node_list.end(), node_id);
  if (it == offline_node_list.end()) {
    offline_node_list.push_back(node_id);
    ucl_network_management_remove_offline_node_manager();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Publication functions
//////////////////////////////////////////////////////////////////////////////
sl_status_t ucl_network_management_remove_offline_node(zwave_node_id_t node_id)
{
  if (node_id == 0x00) {
    return SL_STATUS_FAIL;
  }

  process_post(&ucl_network_management_process,
               UCL_NM_REMOVE_OFFLINE_EVENT,
               new zwave_node_id_t(node_id));

  return SL_STATUS_OK;
}

sl_status_t
  ucl_network_management_request_node_neighbor_update(zwave_node_id_t node_id)
{
  if (node_id == 0x00) {
    return SL_STATUS_FAIL;
  }
  process_post(&ucl_network_management_process,
               UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_EVENT,
               new zwave_node_id_t(node_id));

  return SL_STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////
// Contiki Process
////////////////////////////////////////////////////////////////////////////////
PROCESS(ucl_network_management_process, "ucl_network_management_process");

PROCESS_THREAD(ucl_network_management_process, ev, data)
{
  PROCESS_BEGIN();
  while (true) {
    switch (ev) {
      case PROCESS_EVENT_INIT:
        ucl_network_management_init();
        break;

      case PROCESS_EVENT_EXIT:
        ucl_network_management_exit();
        break;

      case NEW_NETWORK_EVENT:
        ucl_network_management_new_network_ev(
          static_cast<new_network_entered_data *>(data));
        break;
      case MQTT_CB_EVENT: {
        std::string *str_data = static_cast<std::string *>(data);
        write_topic_received(*str_data);
        delete str_data;
      } break;
      case UPDATE_STATE_EVENT: {
        UclNetworkManagementStateData *ucl_nm_state_data
          = static_cast<UclNetworkManagementStateData *>(data);
        state_topic_update(zpc_unid, *ucl_nm_state_data);
        delete ucl_nm_state_data;
      } break;
      case UCL_NM_REMOVE_OFFLINE_EVENT: {
        zwave_node_id_t *node_id = static_cast<zwave_node_id_t *>(data);
        ucl_network_management_trigger_remove_offline_node(*node_id);
        delete node_id;
      } break;
      case PROCESS_EVENT_TIMER: {
        if (!offline_node_list.empty()) {
          ucl_network_management_remove_offline_node_manager();
        } else {
          etimer_stop(&offline_node_timer);
        }
      } break;
      case UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_EVENT: {
        zwave_node_id_t *node_id = static_cast<zwave_node_id_t *>(data);
        ucl_nm_trigger_node_neighbor_update(*node_id);
        delete node_id;
      } break;
      default:
        sl_log_warning(LOG_TAG, "Unhandled event %d", ev);
        break;
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
