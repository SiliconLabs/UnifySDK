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

/**
 * @file zigpc_ucl_fixt.h
 * @defgroup zigpc_ucl ZigPC UCL Utility Component
 * @ingroup zigpc_components
 * @brief This is responsible for publishing and subscribing to Zigbee Protocol
 * Controller specific topics that is not exposed by the UIC shared component.
 *
 * These topics are (currently):
 *  - ucl/by-unid/PC_UNID/ProtocolController/NetworkManagement
 *  - ucl/by-unid/PC_UNID/ProtocolController/NetworkManagement/Write
 *
 *
 * - NetworkManagement State publishing: This functionality update the MQTT topic
 *   when the Network Management FSM changes its state. For example, if the network
 *   management FSM is in the idle state, the MQTT topic
 *   ...ProtocolController/NetworkManagement will be published with {state: "idle"}
 *   as well as other JSON fields that notify of next possible actions that the FSM
 *   can take
 *
 * @{
 */

#ifndef ZIGPC_UCL_HPP
#define ZIGPC_UCL_HPP

#include <string>

// Shared UIC includes
#include <sl_status.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>

/**
 * @brief UCL topic and payload manipulators in the Zigbee Protocol Controller.
 *
 */
namespace zigpc_ucl
{
/**
 * @brief UCL utility functions available to build and parse MQTT topics and
 * payloads.
 *
 */
namespace mqtt
{
/**
 * @brief Types of specific UCL topics that can be build along with variables
 * passed via mqtt::topic_data_t.
 *
 */
enum class topic_type_t {
  BY_UNID_PC_NWMGMT,       /**< Uses topic_data_t fields: eui64 */
  BY_UNID_PC_NWMGMT_WRITE, /**< Uses topic_data_t fields: eui64 */
  BY_UNID_NODE_STATE,      /**< Uses topic_data_t fields: eui64 */
  BY_UNID_NODE,            /**< Uses topic_data_t fields: eui64 */
  BY_UNID_NODE_EP,         /**< Uses topic_data_t fields: eui64, endpoint_id */
};

/**
 * @brief Data that is used to build UCL topics.
 *
 */
typedef struct {
  zigbee_eui64_uint_t eui64;
  zigbee_endpoint_id_t endpoint_id;
} topic_data_t;

/**
 * @brief Populate the topic string based on the topic type and data passed
 * in.
 *
 * @param topic_type    Topic type.
 * @param topic_data    Data to populate in the topic.
 * @param topic         Topic to populate.
 * @return sl_status_t SL_STATUS_OK on succes, or SL_STATUS_INVALID_TYPE on
 * invalid topic type passed in.
 */
sl_status_t build_topic(zigpc_ucl::mqtt::topic_type_t topic_type,
                        zigpc_ucl::mqtt::topic_data_t topic_data,
                        std::string &topic);

/**
 * @brief Parse a UCL UNID into Zigbee device identifier.
 *
 * @param unid          UNID string to parse.
 * @param eui64         Zigbee identifier reference to poulate.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_INVALID_SIGNATURE
 * if the does not contain the configured UNID format (including prefix).
 */
sl_status_t parse_unid(std::string unid, zigbee_eui64_uint_t &eui64);

/**
 * @brief Build a UCL UNID based on the Zigbee device identifier.
 *
 * @param eui64         Zigbee identifier reference to convert.
 * @return std::string  UCL UNID including ZigPC prefix.
 */
std::string build_unid(zigbee_eui64_uint_t eui64);

}  // namespace mqtt

namespace node_state
{
/**
 * @brief Update the device status with the parameters provided. This handler will
 * publish the updated device state on MQTT.
 *
 * @param eui64             Device identifier.
 * @param network_status    Device network status.
 * @param max_cmd_delay     Device maximum command delay.
 * @return sl_status_t      SL_STATUS_OK on success, or MQTT API error
 * otherwise.
 */
sl_status_t publish_state(zigbee_eui64_uint_t eui64,
                          zigbee_node_network_status_t network_status,
                          uint32_t max_cmd_delay);

/**
 * @brief Unretain all topic for a particular device.
 *
 * @param eui64             Device identifier.
 * @return sl_status_t      SL_STATUS_OK on success, or MQTT API error
 * otherwise.
 */
sl_status_t remove_node_topics(zigbee_eui64_uint_t eui64);

/**
 * @brief Unretain all node topics except the node state topic.
 *
 * NOTE: This function should only be run on shutdown.
 *
 */
void cleanup_all_node_topics(void);

}  // namespace node_state

}  // namespace zigpc_ucl

#endif /* ZIGPC_UCL_HPP */

/** @} end zigpc_ucl */
