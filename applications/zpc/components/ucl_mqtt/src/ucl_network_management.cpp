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
#include "ucl_network_management.h"
#include "ucl_nm_neighbor_discovery.h"

// ZPC includes
#include "zpc_converters.h"
#include "zwave_controller.h"
#include "zwave_controller_callbacks.h"
#include "zwave_network_management.h"
#include "zwave_unid.h"

// Contiki
#include "process.h"
#include "sys/etimer.h"

// Unify includes
#include "uic_mqtt.h"
#include "sl_log.h"
#include "sl_status.h"

// Generic includes
#include <boost/iostreams/stream.hpp>
#include <boost/property_tree/ptree.hpp>
#include <nlohmann/json.hpp>
#include <cstddef>
#include <map>
#include <sstream>
#include <deque>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
// Defines and types
////////////////////////////////////////////////////////////////////////////////
#define LOG_TAG "ucl_network_management"

////////////////////////////////////////////////////////////////////////////////
// Contiki Process
////////////////////////////////////////////////////////////////////////////////
PROCESS(ucl_network_management_process, "ucl_network_management_process");

namespace bpt = boost::property_tree;

/**
 * @brief @ref Contiki Event definitions for the Network Monitor Process.
 */
typedef enum {
  /// A new node was added to the network
  NODE_ADDED_EVENT,
  /// Node interview done
  NODE_DELETED_EVENT,
  ///
  MQTT_CB_EVENT,
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
  UCL_NM_TOPIC_NETWORK_UPDATE,
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
  const std::map<std::string, std::string, std::less<>> state_parameters;
  const std::vector<std::string> requested_state_parameters;

  /**
   * @brief Construct a new Unify State Topic Update Data object
   *
   * @param state
   * @param state_parameters
   * @param requested_state_parameters
   */
  UclNetworkManagementStateData(
    ucl_network_management_state_t state,
    const std::map<std::string, std::string, std::less<>> &state_parameters,
    const std::vector<std::string> &requested_state_parameters) :
    state(state),
    state_parameters(state_parameters),
    requested_state_parameters(requested_state_parameters)
  {}

  /**
   * @brief Construct a new Unify State Topic Update Data object
   *
   * @param state
   * @param state_parameters
   */
  UclNetworkManagementStateData(
    ucl_network_management_state_t state,
    const std::map<std::string, std::string, std::less<>> &state_parameters) :
    UclNetworkManagementStateData(
      state, state_parameters, std::vector<std::string>())
  {}

  /**
   * @brief Construct a new Unify State Topic Update Data object
   *
   * @param state
   */
  explicit UclNetworkManagementStateData(ucl_network_management_state_t state,
                                         const std::vector<std::string> &) :
    UclNetworkManagementStateData(
      state,
      std::map<std::string, std::string, std::less<>>(),
      requested_state_parameters)

  {}

  /**
   * @brief Construct a new Unify State Topic Update Data object
   *
   * @param state
   */
  explicit UclNetworkManagementStateData(ucl_network_management_state_t state) :
    UclNetworkManagementStateData(
      state,
      std::map<std::string, std::string, std::less<>>(),
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

static void ucl_network_management_mqtt_callback(const char *topic,
                                                 const char *message,
                                                 const size_t message_length);

/**
 * @brief Builds a full MQTT topic prefix with our current UNID.
 *
 * @param unid            The UNID of the Z-Wave Controller
 * @return std::string topic prefix for NetworkManagement.
 */
static std::string get_topic(const unid_t unid);

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

static void
  ucl_network_management_on_network_address_update(zwave_home_id_t home_id,
                                                   zwave_node_id_t node_id);

static void
  ucl_network_management_on_node_added(sl_status_t status,
                                       const zwave_node_info_t *nif,
                                       zwave_node_id_t node_id,
                                       const zwave_dsk_t dsk,
                                       zwave_keyset_t granted_keys,
                                       zwave_kex_fail_type_t kex_fail_type,
                                       zwave_protocol_t inclusion_protocol);

// Contiki event helper handler functions
static void ucl_network_management_init();
static void ucl_network_management_exit();

////////////////////////////////////////////////////////////////////////////////
// Local Variables and Consts
////////////////////////////////////////////////////////////////////////////////
constexpr std::string_view UCL_NM_TOPIC_IDLE_STR           = "idle";
constexpr std::string_view UCL_NM_TOPIC_ADD_NODE_STR       = "add node";
constexpr std::string_view UCL_NM_TOPIC_REMOVE_NODE_STR    = "remove node";
constexpr std::string_view UCL_NM_TOPIC_JOIN_NETWORK_STR   = "join network";
constexpr std::string_view UCL_NM_TOPIC_LEAVE_NETWORK_STR  = "leave network";
constexpr std::string_view UCL_NM_TOPIC_NETWORK_REPAIR_STR = "network repair";
constexpr std::string_view UCL_NM_TOPIC_NETWORK_UPDATE_STR = "network update";
constexpr std::string_view UCL_NM_TOPIC_RESET_STR          = "reset";
constexpr std::string_view UCL_NM_TOPIC_TEMPORARILY_OFFLINE_STR = "idle";
constexpr std::string_view UCL_NM_TOPIC_SCAN_MODE_STR           = "scan mode";

// Use boost::property_tree::detail::less_nocase as comparator
// to make map keys case insensitive
static const std::multimap<std::string_view,
                           ucl_network_management_state_t,
                           bpt::detail::less_nocase<std::string_view>>
  ucl_network_management_state_map = {
    {UCL_NM_TOPIC_IDLE_STR, UCL_NM_TOPIC_IDLE},
    {UCL_NM_TOPIC_ADD_NODE_STR, UCL_NM_TOPIC_ADD_NODE},
    {UCL_NM_TOPIC_REMOVE_NODE_STR, UCL_NM_TOPIC_REMOVE_NODE},
    {UCL_NM_TOPIC_JOIN_NETWORK_STR, UCL_NM_TOPIC_JOIN_NETWORK},
    {UCL_NM_TOPIC_LEAVE_NETWORK_STR, UCL_NM_TOPIC_LEAVE_NETWORK},
    {UCL_NM_TOPIC_NETWORK_REPAIR_STR, UCL_NM_TOPIC_NETWORK_REPAIR},
    {UCL_NM_TOPIC_NETWORK_UPDATE_STR, UCL_NM_TOPIC_NETWORK_UPDATE},
    {UCL_NM_TOPIC_RESET_STR, UCL_NM_TOPIC_RESET},
    {UCL_NM_TOPIC_TEMPORARILY_OFFLINE_STR, UCL_NM_TOPIC_TEMPORARILY_OFFLINE},
    {UCL_NM_TOPIC_SCAN_MODE_STR, UCL_NM_TOPIC_SCAN_MODE},
};

static const zwave_controller_callbacks_t ucl_network_management_callbacks = {
  .on_state_updated = &ucl_network_management_on_state_updated,
  .on_node_added    = &ucl_network_management_on_node_added,
  .on_network_address_update
  = &ucl_network_management_on_network_address_update,
  .on_keys_report = &ucl_network_management_on_keys_report,
  .on_dsk_report  = &ucl_network_management_on_dsk_report,
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
  ucl_network_management_state_t ucl_nm_state = UCL_NM_TOPIC_LAST;
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
    case NM_ASSIGNING_RETURN_ROUTE:
    case NM_NEIGHBOR_DISCOVERY_ONGOING:
      ucl_nm_state = UCL_NM_TOPIC_NETWORK_REPAIR;
      break;
    default:
      sl_log_warning(LOG_TAG,
                     "Unhandled Network Management state: %d",
                     nm_state);
      break;
  }

  if (ucl_nm_state != UCL_NM_TOPIC_LAST) {
    state_topic_update(zpc_unid, UclNetworkManagementStateData(ucl_nm_state));
  }
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
                 "zwave_network_management_keys_set failed with Status: 0x%04X",
                 status);
  }
}

static void
  ucl_network_management_on_network_address_update(zwave_home_id_t home_id,
                                                   zwave_node_id_t node_id)
{
  // UNID just changed, we un-subscribe, re-subscribe:
  uic_mqtt_unretain((get_topic(zpc_unid) + "/NetworkManagement").c_str());
  write_topic_unsubscribe(zpc_unid, ucl_network_management_mqtt_callback);

  // Set up the new network:
  zwave_unid_set_home_id(home_id);
  zwave_unid_from_node_id(node_id, zpc_unid);
  write_topic_subscribe(zpc_unid, ucl_network_management_mqtt_callback);
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
      sl_log_info(LOG_TAG,
                  "Invalid DSK input length (%d). The number of missing bytes "
                  "for the DSK input must be an even number. Please try again",
                  input_length);
      return;
    }
    // More input validation of 'input_length'
    if (input_length > ZWAVE_DSK_LENGTH) {
      sl_log_info(LOG_TAG,
                  "Invalid DSK input. You have entered too much data (%d), "
                  "more than the length of a DSK (%d). Please try again",
                  input_length,
                  ZWAVE_DSK_LENGTH);

      return;
    }
    if (zpc_converters_dsk_to_str(dsk, dsk_str, sizeof(dsk_str))
        != SL_STATUS_OK) {
      sl_log_info(LOG_TAG, "Failed to convert DSK to string");
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

    state_topic_update(
      zpc_unid,
      UclNetworkManagementStateData(
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
                                       const zwave_dsk_t dsk,
                                       zwave_keyset_t granted_keys,
                                       zwave_kex_fail_type_t kex_fail_type,
                                       zwave_protocol_t inclusion_protocol)
{
  // When node addition is completed, we are back to IDLE and ready to carry
  // New operations.
  state_topic_update(zpc_unid,
                     UclNetworkManagementStateData(UCL_NM_TOPIC_IDLE));
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

/**
 * @brief Helper function for getting the json value with case-insensitive key. I.e. :
 * @code
 * {
 *   "StAtE": "aDd nOdE",
 *   "StaTeParamEtErs": {
 *     "UsErAcCepT": true,
 *     "SecuRItyCOde" : "12345-44510-61374-04097-44510-61374-61183-00256",
 *     "AllowMultipleInclusions": true
 *   }
 * }
 * @endcode
 * Using this function you may get the "StAtE" value even if passed key differs in letters cases:
 * @code
 * nlohmann::json state_json = get_case_insensitive_json (src_json, std::string("state"));
 * @endcode
 * If there is not a value with a passed key - the empty nlohmann::json object is returned.
 *
 * @param[in] jsn already parsed json object
 * @param[in] key the key of the value, we are searching for
 *
 * @return nlohmann::json object with data if OK, empty nlohmann::json object in a case of failure.
 */
static nlohmann::json get_case_insensitive_json(const nlohmann::json &jsn,
                                                const std::string &key)
{
  auto jsn_map = jsn.get<std::map<std::string,
                                  nlohmann::json,
                                  bpt::detail::less_nocase<std::string>>>();

  auto res_iter = jsn_map.find(key);

  if (jsn_map.end() == res_iter) {
    nlohmann::json empty_jsn;
    return empty_jsn;
  }

  return res_iter->second;
}

static sl_status_t write_topic_received(const std::string &message)
{
  nlohmann::json jsn;

  try {
    jsn = nlohmann::json::parse(message);
  } catch (const nlohmann::json::exception &err) {
    sl_log_debug(LOG_TAG,
                 "Failed to parse JSON message: '%s', error: %s",
                 message.c_str(),
                 err.what());
    return SL_STATUS_FAIL;
  }

  try {
    std::string state_msg
      = get_case_insensitive_json(jsn, std::string("State"));
    sl_log_debug(LOG_TAG, "State: '%s' received", state_msg.c_str());

    auto state = ucl_network_management_state_map.find(state_msg);
    if (state == ucl_network_management_state_map.end()) {
      sl_log_debug(LOG_TAG,
                   "Invalid state: %s, ignoring command.",
                   state_msg.c_str());
      return SL_STATUS_FAIL;
    }

    switch (state->second) {
      case UCL_NM_TOPIC_IDLE:
        allow_multiple_inclusions = false;
        zwave_network_management_abort();
        break;
      case UCL_NM_TOPIC_ADD_NODE: {
        nlohmann::json jsn_state_param
          = get_case_insensitive_json(jsn, std::string("StateParameters"));

        if (
          jsn_state_param
            .is_null()) {  // User have not supplied StateParameters, assuming add node
          zwave_network_management_add_node();
          break;
        }

        // User have supplied StateParameters
        nlohmann::json jsn_allow_mult_inc
          = get_case_insensitive_json(jsn_state_param,
                                      std::string("AllowMultipleInclusions"));
        nlohmann::json jsn_user_accept
          = get_case_insensitive_json(jsn_state_param,
                                      std::string("UserAccept"));
        nlohmann::json jsn_secur_code
          = get_case_insensitive_json(jsn_state_param,
                                      std::string("SecurityCode"));

        if (!jsn_allow_mult_inc.is_null()) {
          allow_multiple_inclusions = jsn_allow_mult_inc;
        }

        if (jsn_user_accept.is_null()) {  // UserAccept have not been supplied
          zwave_network_management_add_node();
          break;
        }

        // User have supplied UserAccepted
        bool user_accept = jsn_user_accept;

        if (user_accept) {  // UserAccepted is true
          sl_log_debug(LOG_TAG, "User accepted node add");

          if (
            jsn_secur_code
              .is_null()) {  // UserAccepted is true, but SecurityCode is not supplied
            sl_log_debug(
              LOG_TAG,
              "\"State\": \"add node\" is missing \"StateParameters\" "
              "\"SecurityCode\", discarding command: %s",
              message.c_str());
            break;
          }

          // User have supplied SecurityCode
          std::string dsk_str = jsn_secur_code;
          zwave_dsk_t zwave_dsk;

          if (SL_STATUS_OK
              == zpc_converters_dsk_str_to_internal(dsk_str.c_str(),
                                                    zwave_dsk)) {
            zwave_network_management_dsk_set(zwave_dsk);
          } else {  // Failed to parse dsk
            sl_log_debug(LOG_TAG, "Failed to parse DSK: '%s'", dsk_str.c_str());
          }
        } else {  // UserAccepted is false
          sl_log_debug(
            LOG_TAG,
            "User rejected node add operation (UserAccepted = false).");
          zwave_network_management_abort();
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
      case UCL_NM_TOPIC_NETWORK_UPDATE:
        break;
      case UCL_NM_TOPIC_RESET:
        zwave_controller_reset();
        if (true == zwave_controller_is_reset_ongoing()) {
          // Publish immediately that we initiated reset
          state_topic_update(zpc_unid,
                             UclNetworkManagementStateData(UCL_NM_TOPIC_RESET));
        }
        break;
      case UCL_NM_TOPIC_TEMPORARILY_OFFLINE:
        break;
      case UCL_NM_TOPIC_SCAN_MODE:
        break;
      case UCL_NM_TOPIC_LAST:
        break;
      default:
        sl_log_warning(LOG_TAG,
                       "Unhandled UCL Network Management Topic: %d",
                       state->second);
        break;
    }
  } catch (const nlohmann::json::exception &err) {
    sl_log_error(LOG_TAG,
                 "JSON payload invalid %s, Error: %s",
                 message.c_str(),
                 err.what());
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

nlohmann::json get_supported_states(ucl_network_management_state_t state)
{
  switch (state) {
    case UCL_NM_TOPIC_IDLE:
      return nlohmann::json::array({UCL_NM_TOPIC_ADD_NODE_STR,
                                    UCL_NM_TOPIC_REMOVE_NODE_STR,
                                    UCL_NM_TOPIC_RESET_STR});

    case UCL_NM_TOPIC_ADD_NODE:
      return nlohmann::json::array({UCL_NM_TOPIC_IDLE_STR});
    case UCL_NM_TOPIC_REMOVE_NODE:
      return nlohmann::json::array({UCL_NM_TOPIC_IDLE_STR});

    case UCL_NM_TOPIC_REMOVE_FAILED_NODE:
      return nlohmann::json::array({UCL_NM_TOPIC_IDLE_STR});
    case UCL_NM_TOPIC_JOIN_NETWORK:
      return nlohmann::json::array({UCL_NM_TOPIC_IDLE_STR});
    case UCL_NM_TOPIC_LEAVE_NETWORK:
      break;
    case UCL_NM_TOPIC_NODE_INTERVIEW:
      break;
    case UCL_NM_TOPIC_NETWORK_REPAIR:
      return nlohmann::json::array({UCL_NM_TOPIC_IDLE_STR});
    case UCL_NM_TOPIC_NETWORK_UPDATE:
      break;
    case UCL_NM_TOPIC_RESET:
      break;
    case UCL_NM_TOPIC_TEMPORARILY_OFFLINE:
      break;
    case UCL_NM_TOPIC_SCAN_MODE:
      return nlohmann::json::array({UCL_NM_TOPIC_IDLE_STR});
    default:
      break;
  }
  return nlohmann::json::array();
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
      nlohmann::json root;
      // State
      root["State"] = it->first;

      // SupportedStateList
      root["SupportedStateList"] = get_supported_states(it->second);

      // StateParameters
      if (!uic_state_topic_update_data.state_parameters.empty()) {
        root["StateParameters"] = uic_state_topic_update_data.state_parameters;
      }

      // RequestedStateParamters
      if (!uic_state_topic_update_data.requested_state_parameters.empty()) {
        root["RequestedStateParameters"]
          = uic_state_topic_update_data.requested_state_parameters;
      }

      std::string msg = root.dump();
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

  // Make sure UNID is configured.
  zwave_unid_set_home_id(zwave_network_management_get_home_id());
  zwave_unid_from_node_id(zwave_network_management_get_node_id(), zpc_unid);
  write_topic_subscribe(zpc_unid, ucl_network_management_mqtt_callback);
  ucl_network_management_on_state_updated(zwave_network_management_get_state());
  /// intialize ucl network management node neighbor discovery submodule
  ucl_nm_neighbor_discovery_init();
}

static void ucl_network_management_exit()
{
  UclNetworkManagementStateData ucl_nm_state_data(
    UCL_NM_TOPIC_TEMPORARILY_OFFLINE);
  state_topic_update(zpc_unid, ucl_nm_state_data);
  write_topic_unsubscribe(zpc_unid, ucl_network_management_mqtt_callback);
  uic_mqtt_unretain((get_topic(zpc_unid) + "/NetworkManagement").c_str());
}

static void ucl_network_management_remove_offline_node_manager()
{
  if (zwave_network_management_get_state() == NM_IDLE) {
    zwave_network_management_remove_failed(offline_node_list.front());
    offline_node_list.pop_front();
  } else {
    sl_log_debug(LOG_TAG,
                 "Network management state is not idle. Will retry the Remove "
                 "Offline operation of Node: %d in a second \n",
                 offline_node_list.front());
  }
  if (!offline_node_list.empty()) {
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
    sl_log_debug(LOG_TAG,
                 "NodeID %d NOT found in offline node list. Adding it\n",
                 node_id);
    offline_node_list.push_back(node_id);
    ucl_network_management_remove_offline_node_manager();
  } else {
    sl_log_debug(
      LOG_TAG,
      "NodeID %d is already in offline node list. It will be removed shortly",
      node_id);
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

      case PROCESS_EVENT_EXITED:
        // Do not do anything with this event, just wait to go down.
        break;

      case MQTT_CB_EVENT: {
        std::string *str_data = static_cast<std::string *>(data);
        write_topic_received(*str_data);
        delete str_data;
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
          sl_log_debug(LOG_TAG, "Stopping the offline node timer\n");
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
