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

#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>

// Unify shared includes
#include <sl_log.h>

// ZigPC includes
#include <zigpc_datastore.h>

// Component includes
#include "zigpc_ucl_int.hpp"


/**
 * @brief Map between the Network Management FSM states and the
 * next states supported. This map is used by the component
 * to pass State information though MQTT publishing.
 *
 */
static const std::unordered_map<zigpc_net_mgmt_fsm_state_t, std::string>
  STATE_TYPE_MAP = {
    {ZIGPC_NET_MGMT_FSM_STATE_IDLE, "idle"},
    {ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD, "add node"},
    {ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE, "remove node"},
    {ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW, "node interview"},
};

// JSON key labels to be used
static constexpr char KEY_STATE[]            = "State";
static constexpr char KEY_STATE_LIST[]       = "SupportedStateList";
static constexpr char KEY_STATE_PARAMETERS[] = "StateParameters";
static constexpr char KEY_REQ_STATE_PARAMETER_LIST[]
  = "RequestedStateParameters";
static constexpr char KEY_STATE_PARAMETER_UNID[] = "Unid";

/**
 * @brief Helper to parse requested state from MQTT JSON payload into the
 * state change request object.
 *
 * @param payload_jsn   JSON payload parsed as nlohmann::json object.
 * @param state_change  State change object to poulate.
 * @return sl_status_t  SL_STATUS_OK on success,
 * SL_STATUS_INVALID_CONFIGURATION on missing state field, or
 * SL_STATUS_INVALID_TYPE on unknown state label parsed.
 */
static sl_status_t helper_parse_requested_state(
  nlohmann::json &payload_jsn, zigpc_net_mgmt_state_change_request_t &state_change)
{
  sl_status_t status = SL_STATUS_OK;

  if (payload_jsn.count(KEY_STATE) == 0) {
    status = SL_STATUS_INVALID_CONFIGURATION;
    sl_log_warning(zigpc_ucl::LOG_TAG,
                   "Required State field missing from message: 0x%X",
                   status);
    return status;
  }
  
  std::string state_field = ""; 
  try
  {
    state_field  = payload_jsn[KEY_STATE].get<std::string>();
  }
  catch(const nlohmann::json::exception& e)
  {
    status = SL_STATUS_OBJECT_READ;
    sl_log_warning(zigpc_ucl::LOG_TAG,
                   "Failed to parse the JSON payload: 0x%X",
                   status);
    return status;
  }

  for (auto &state_entry: STATE_TYPE_MAP) {
    if (state_entry.second == state_field) {
      state_change.requested_state = state_entry.first;
      break;
    }
  }

  if (state_change.requested_state == ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL) {
    status = SL_STATUS_INVALID_TYPE;
    sl_log_warning(zigpc_ucl::LOG_TAG,
                    "Unknown state requested: 0x%X",
                    status);
  } else {
    sl_log_debug(zigpc_ucl::LOG_TAG,
                  "Requesting state change to '%s'",
                  state_field.c_str());
  }

  return status;
}

/**
 * @brief Helper to parse recognized state paramters from MQTT JSON payload
 * into the state change request object.
 *
 * @param payload_jsn   JSON payload parsed as nlohmann::json object.
 * @param state_change  State change object to poulate.
 * @return sl_status_t  SL_STATUS_OK on success,
 * SL_STATUS_INVALID_SIGNATURE if the Unid parameter is not recognized.
 */
static sl_status_t helper_parse_state_parameters(
  nlohmann::json &payload_jsn, zigpc_net_mgmt_state_change_request_t &state_change)
{
  sl_status_t status = SL_STATUS_OK;

  if (payload_jsn.count(KEY_STATE_PARAMETERS) == 0 || payload_jsn[KEY_STATE_PARAMETERS].is_null()) {
    return status;
  }

  nlohmann::json state_parameters_obj = payload_jsn[KEY_STATE_PARAMETERS];

  // Extract state parameters object if populated
  if (state_parameters_obj.count(KEY_STATE_PARAMETER_UNID) == 0) {
    return status;
  }

  std::string unid_field = ""; 
  try
  {
    unid_field = state_parameters_obj[KEY_STATE_PARAMETER_UNID].get<std::string>();
  }
  catch(const nlohmann::json::exception& e)
  {
    status = SL_STATUS_OBJECT_READ;
    sl_log_warning(zigpc_ucl::LOG_TAG,
                   "Failed to parse the JSON payload: 0x%X",
                   status);
    return status;
  }

  zigbee_eui64_uint_t eui64;
  status = zigpc_ucl::mqtt::parse_unid(unid_field, eui64);
  status |= zigbee_uint_to_eui64(eui64, state_change.param_eui64);
  if (status != SL_STATUS_OK) {
    sl_log_warning(zigpc_ucl::LOG_TAG,
                    "Unable to parse UNID: 0x%X",
                    status);
  } else {
    state_change.param_eui64_filled = true;
  }

  return status;
}

/**
 * @brief MQTT subscription handler for receiving network management
 * updates. This function will send a dynamically allocated string from the
 * character array passed in as a the MQTT message.
 *
 * @param topic           Character array containing MQTT topic triggered
 * @param message         Character array containing MQTT payload
 * @param message_length  Length of MQTT payload
 */
void zigpc_ucl::pc_nwmgmt::on_write_mqtt(const char *topic,
                                         const char *message,
                                         const size_t message_length)
{
  sl_status_t status = SL_STATUS_OK;
  nlohmann::json payload_jsn;

  if (message == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    status = zigpc_ucl::mqtt::parse_payload(message, payload_jsn);
  }

  if ((status == SL_STATUS_OK) && (payload_jsn.empty())) {
    status = SL_STATUS_EMPTY;
  }

  zigpc_net_mgmt_state_change_request_t state_change = {
    .requested_state    = ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL,
    .param_eui64_filled = false,
  };

  // Extract required state field
  if (status == SL_STATUS_OK) {
    status = helper_parse_requested_state(payload_jsn, state_change);
  }

  if (status == SL_STATUS_OK) {
    status = helper_parse_state_parameters(payload_jsn, state_change);
  }

  if (status == SL_STATUS_OK) {

      //IDLE
      //ADD NODE
      //REMOVE NODE
    status = zigpc_net_mgmt_state_change_request(&state_change);
    if (status != SL_STATUS_OK) {
      sl_log_error(zigpc_ucl::LOG_TAG,
                   "Failed to request state change in network management: 0x%X",
                   status);
    }
  }
  sl_log_debug(zigpc_ucl::LOG_TAG,
               "NetworkManagement/Write handler status: 0x%X",
               status);
}

sl_status_t zigpc_ucl::pc_nwmgmt::on_net_state_update(
  zigpc_net_mgmt_on_network_state_update_t &state)
{
  sl_log_debug(zigpc_ucl::LOG_TAG, "Update network state");
  sl_status_t status = SL_STATUS_OK;

  nlohmann::json payload_jsn;

  // Populate new state
  if (status == SL_STATUS_OK) {
    auto found_new_state = STATE_TYPE_MAP.find(state.new_state);
    if (found_new_state == STATE_TYPE_MAP.end()) {
      status = SL_STATUS_INVALID_TYPE;
      sl_log_warning(zigpc_ucl::LOG_TAG,
                     "Unknown state to publish: 0x%X",
                     status);
    } else {
      payload_jsn[KEY_STATE] = found_new_state->second;
    }
  }

  // Populate next supported state list
  if (status == SL_STATUS_OK) {
    std::vector<zigpc_net_mgmt_fsm_state_t> supported_states(
      state.next_supported_states_list,
      state.next_supported_states_list + state.next_supported_states_count);

    nlohmann::json arr_jsn = nlohmann::json::array();
    for (auto &supported_state: supported_states) {
      auto found_state = STATE_TYPE_MAP.find(supported_state);
      if (found_state == STATE_TYPE_MAP.end()) {
        status = SL_STATUS_INVALID_TYPE;
        sl_log_warning(zigpc_ucl::LOG_TAG,
                       "Unknown next state to publish: 0x%X",
                       status);
      } else {
        arr_jsn.push_back(found_state->second);
      }
    }

    if (arr_jsn.empty()) {
      payload_jsn[KEY_STATE_LIST] = "[]";
    } else {
      payload_jsn[KEY_STATE_LIST] = arr_jsn;
    }
  }

  // Populate requested state parameter list
  if (status == SL_STATUS_OK) {
    std::vector<const char *> req_params(
      state.requested_state_parameter_list,
      state.requested_state_parameter_list
        + state.requested_state_parameter_count);

    nlohmann::json arr_jsn = nlohmann::json::array();
    for (auto &param: req_params) {
      arr_jsn.push_back(std::string(param));
    }

    if (arr_jsn.empty()) {
      payload_jsn[KEY_REQ_STATE_PARAMETER_LIST] = "[]";
    } else {
      payload_jsn[KEY_REQ_STATE_PARAMETER_LIST] = arr_jsn;
    }
  }

  // Get PC EUI64;
  zigpc_ucl::mqtt::topic_data_t topic_data;
  if (status == SL_STATUS_OK) {
    zigpc_network_data_t nwk_data;
    status = zigpc_datastore_read_network(&nwk_data);
    if (status == SL_STATUS_OK) {
      topic_data.eui64 = zigbee_eui64_to_uint(nwk_data.gateway_eui64);
    }
  }

  if (status == SL_STATUS_OK) {

    std::string payload_str = "";
    try
    {
        payload_str = payload_jsn.dump();
    }
    catch(const nlohmann::json::exception& e)
    {
        status = SL_STATUS_OBJECT_READ;
        sl_log_warning(zigpc_ucl::LOG_TAG,
                   "Failed to parse the JSON payload: 0x%X",
                   status);
        return status;
    }

    boost::replace_all(payload_str, "\"[]\"", "[]");
    boost::replace_all(payload_str, "\n", "");

    zigpc_ucl::mqtt::publish(zigpc_ucl::mqtt::topic_type_t::BY_UNID_PC_NWMGMT,
                             topic_data,
                             payload_str.c_str(),
                             payload_str.size(),
                             true);
  }

  return status;
}

sl_status_t zigpc_ucl::pc_nwmgmt::on_net_init(zigbee_eui64_uint_t pc_eui64)
{
      
  sl_log_debug(zigpc_ucl::LOG_TAG, "Subscribe to MQTT at network initialization");
  zigpc_ucl::mqtt::topic_data_t topic_data;

  topic_data.eui64 = pc_eui64;

  sl_status_t status = zigpc_ucl::mqtt::subscribe(
    zigpc_ucl::mqtt::topic_type_t::BY_UNID_PC_NWMGMT_WRITE,
    topic_data,
    zigpc_ucl::pc_nwmgmt::on_write_mqtt);
  
      return status;
}
