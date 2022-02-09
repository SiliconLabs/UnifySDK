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

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

// UIC shared includes
#include <sl_log.h>

// ZigPC includes
#include <zigpc_datastore.h>

// Component includes
#include "zigpc_ucl.hpp"
#include "zigpc_ucl_int.hpp"

/**
 * @brief Map between the internal Node state and the UCL node state labels.
 *
 */
static const std::unordered_map<zigbee_node_network_status_t, std::string_view>
  NODE_STATE_TYPE_MAP = {
    {ZIGBEE_NODE_STATUS_NODEID_ASSIGNED, "Online interviewing"},
    {ZIGBEE_NODE_STATUS_INCLUDED, "Online functional"},
    {ZIGBEE_NODE_STATUS_INTERVIEW_FAIL, "Online non-functional"},
    {ZIGBEE_NODE_STATUS_INTERVIEWING, "Online interviewing"},
    {ZIGBEE_NODE_STATUS_OFFLINE, "Offline"},
    {ZIGBEE_NODE_STATUS_UNAVAILABLE, "Unavailable"},
};

namespace state_key
{
/**
 * @brief Encryption type used for communication on the PAN node
 *
 */
constexpr char SECURITY[] = "Security";
/**
 * @brief Max delay to deliver a command to a node.
 *
 */
constexpr char MAXCMDDELAY[] = "MaximumCommandDelay";
/**
 * @brief Network availability of the node.
 *
 */
constexpr char NWK_STATUS[] = "NetworkStatus";
}  // namespace state_key

/**
 * @brief Topic regex for selecting all UNID topics except the <UNID>/State
 * topic.
 *
 */
constexpr char TOPIC_REGEX_ALL_UNID_EXCEPT_STATE[]
  = "^(?!ucl\\/by-unid\\/.*\\/State$).*";

/**
 * @brief Default PAN Security based on APS-level Profile ID.
 *
 * The default is the Z3 Security profile.
 *
 */
constexpr char DEFAULT_NODE_SECURITY[] = "Zigbee Z3";

sl_status_t zigpc_ucl::node_state::publish_state(
  zigbee_eui64_uint_t eui64,
  zigbee_node_network_status_t network_status,
  uint32_t max_cmd_delay)
{
  sl_status_t status = SL_STATUS_OK;

  if (status == SL_STATUS_OK) {
    nlohmann::json payload;

    payload[state_key::SECURITY] = DEFAULT_NODE_SECURITY;

    payload[state_key::MAXCMDDELAY] = max_cmd_delay;

    const auto &node_state = NODE_STATE_TYPE_MAP.find(network_status);
    if (node_state != NODE_STATE_TYPE_MAP.end()) {
      payload[state_key::NWK_STATUS] = node_state->second;
    } else {
      payload[state_key::NWK_STATUS]
        = NODE_STATE_TYPE_MAP.at(ZIGBEE_NODE_STATUS_UNAVAILABLE);
    }

    std::string payload_str = payload.dump();

    zigpc_ucl::mqtt::topic_data_t topic_data;
    topic_data.eui64 = eui64;

    status = zigpc_ucl::mqtt::publish(
      zigpc_ucl::mqtt::topic_type_t::BY_UNID_NODE_STATE,
      topic_data,
      payload_str.c_str(),
      payload_str.size(),
      true);
  }

  return status;
}

sl_status_t zigpc_ucl::node_state::remove_node_topics(zigbee_eui64_uint_t eui64)
{
  zigpc_ucl::mqtt::topic_data_t topic_data;
  topic_data.eui64 = eui64;

  sl_status_t status
    = zigpc_ucl::mqtt::unretain(zigpc_ucl::mqtt::topic_type_t::BY_UNID_NODE,
                                topic_data);

  return status;
}

void zigpc_ucl::node_state::cleanup_all_node_topics(void)
{
  //Remove all retained topics except ucl/by-unid/<xxxxx>/State
  uic_mqtt_unretain_by_regex(TOPIC_REGEX_ALL_UNID_EXCEPT_STATE);
}
