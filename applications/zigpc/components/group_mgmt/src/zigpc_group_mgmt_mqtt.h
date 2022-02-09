/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file group_mgmt_mqtt.h
 * @defgroup zigpc_group_mgmt_mqtt ZigPC Group Management MQTT functions
 * @ingroup zigpc_group_mgmt
 * @brief This header contains the definitions and functionality associated with
 * handling groups MQTT functionality. Functions for publishing to MQTT and helpers
 * are in this module
 *
 * @{
 */

#ifndef ZIGPC_GROUP_MGMT_MQTT_H
#define ZIGPC_GROUP_MGMT_MQTT_H

#include "zigpc_common_zigbee.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief zigpc_group_mqttpub_republish
 *
 * Republishes the grouplist for every available node
 *
 * @return SL_STATUS_OK if able to properly republish
 * the grouplist
 *
 **/
sl_status_t
    zigpc_group_mqttpub_republish();

/**
 * @brief zigpc_group_mqttpub_grouplist
 * Publish on the MQTT groups list topic for either REPORTED or DESIRED
 *
 * @param eui64         - the eui64 of the node
 * @param endpoint_id   - the endpoint on the group
 * @param is_reported   - the direction (REPORTED/DESIRED)
 *
 * @return SL_STATUS_OK if the grouplist could be published successfully 
 */
sl_status_t
  zigpc_group_mqttpub_grouplist(const zigbee_eui64_t eui64,
                                const zigbee_endpoint_id_t endpoint_id,
                                bool is_reported);

/**
 * @brief zigpc_group_mqttpub_member_groupname 
 * Publish the name of a group for a given member
 *
 * @param eui64         - the eui64 of the node
 * @param endpoint_id   - the endpoint on the group
 * @param group_id      - the group_id
 * @param is_reported   - the direction (REPORTED/DESIRED)
 *
 * @return SL_STATUS_OK if the grouplist could be published successfully 
 */
sl_status_t
  zigpc_group_mqttpub_member_groupname(const zigbee_eui64_t eui64,
                                       const zigbee_endpoint_id_t endpoint_id,
                                       const zigbee_group_id_t group_id,
                                       bool is_reported);

/**
 * @brief zigpc_group_mqtt_assemble_topic
 *
 * @param eui64         - the eui64 of the node 
 * @param endpoint_id   - the endpoint on the group
 * @param base_cstr     - the base topic string
 * @param base_cstr_size- the size of the base topic string
 * @param is_reported   - the direction (REPORTED/DESIRED)
 * @param topic_string  - the destination where to put the topic string
 * @param topic_size    - the size of topic_string
 *
 * @return SL_STATUS_OK if the payload is assembled, SL_STATUS_WOULD_OVERFLOW
 * if the passed in payload character buffer is insufficient.
 */
sl_status_t
  zigpc_group_mqtt_assemble_topic(const zigbee_eui64_t eui64,
                                  const zigbee_endpoint_id_t endpoint_id,
                                  const char *base_cstr,
                                  unsigned int base_cstr_size,
                                  bool is_reported,
                                  char *topic_string,
                                  unsigned int topic_size);

/**
 * @brief Assemble the JSON payload for GroupList attribute. If there are no
 * groupIDs part of a EUI64/endpoint identifer, the payload will be an empty
 * string with payload_size set to 0U; else the payload will be in an array
 * format and the payload_size variable will be set to the resulting payload
 * string size.
 *
 * @param eui64         - the eui64 of the node
 * @param endpoint_id   - the endpoint on the group
 * @param is_reported   - the direction (REPORTED/DESIRED)
 * @param payload_string- the destination where to put the payload
 * @param payload_size  - input: the capacity of the payload_string, output:
 *                        size of the resulting payload assembled.
 *
 * @return SL_STATUS_OK if the payload is assembled, SL_STATUS_WOULD_OVERFLOW
 * if the passed in payload character buffer is insufficient.
 */
sl_status_t zigpc_group_mqtt_assemble_group_payload(
  const zigbee_eui64_t eui64,
  const zigbee_endpoint_id_t endpoint_id,
  bool is_reported,
  char *payload_string,
  size_t *payload_size);

/**
 * @brief zigpc_group_mqtt_assemble_name_payload
 * Assemble the payload for publishing a group name. For empty group names, the
 * payload with the value formatting is still populated. For group names not
 * found, an empty string is returned
 * @param eui64             - the eui64 of the endpoint
 * @param endpoint_id       - the endpoint with a groups cluster
 * @param group_id          - the group id with a name
 * @param payload_string    - the destination where to copy the payload
 * @param payload_size      - the size of payload_string IN: Capacity, OUT:
 *                            number of characters populated.
 *
 * @return SL_STATUS_OK on valid payload population for group names in either
 * non-empty, empty, or not-found states, SL_STATUS_NULL_POINTER on invalid
 * args passed in, or SL_STATUS_WOULD_OVERFLOW if passed in payload buffer is
 * insufficient.
 */
sl_status_t
  zigpc_group_mqtt_assemble_name_payload(const zigbee_eui64_t eui64,
                                         const zigbee_endpoint_id_t endpoint_id,
                                         const zigbee_group_id_t group_id,
                                         bool is_reported,
                                         char *const payload_string,
                                         size_t *const payload_size);

/** @} end zigpc_group_mgmt_mqtt */
#ifdef __cplusplus
}
#endif
#endif  //ZIGPC_GROUP_MGMT_MQTT_H
