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
// Includes from this component
#include "zwave_tx_scheme_selector.h"

// Generic includes
#include <stdlib.h>

// ZPC Includes
#include "zwave_unid.h"
#include "zpc_attribute_store_network_helper.h"
#include "zpc_attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_network_management.h"
#include "zwave_controller_keyset.h"
#include "zwave_tx_groups.h"  // to use ZWAVE_TX_INVALID_GROUP
#include "zwapi_protocol_basis.h"

// UIC Includes
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "sl_log.h"
#define LOG_TAG "zwave_tx_scheme_selector"

// PHY/MAC Constants
/// Maximum MSDU size for R1/R2 (aMacMaxMSDUSizeR1 / aMacMaxMSDUSizeR2)
#define MAX_MSDU_SIZE_R1_R2 54
/// Maximum MSDU size for R3 (aMacMaxMSDUSizeR3)
#define MAX_MSDU_SIZE_R3 158
/// Maximum MSDU size for Z-Wave Long Range channels
/// Here we need to take (aPhyMaxFrameSizeLR1 - MAC header size(12) - FCS(2) - Extension control (1) - maximum extension (7))
#define MAX_MSDU_SIZE_LONG_RANGE 192 - 12 - 2 - 1 - 7
/// Maximum Routing header size for R1/R2
/// (Routing header + Wake Up extension). Normal Explore Frame is also 8
#define MAX_ROUTING_HEADER_SIZE_R1_R2 8
/// Maximum Routing header size for R3
/// (Routing header + Wake Up extension). Normal Explore Frame is also 8
#define MAX_ROUTING_HEADER_SIZE_R3 8
/// Maximum Routing header size for Z-Wave Long Range channels... No routing here
/// TBD: Should we still set that to 8, so we allow for future routing?
#define MAX_ROUTING_HEADER_SIZE_LONG_RANGE 0

// Encapsulation overhead Constants
/// Number of bytes used by an CRC-16 encapsulation message
#define CRC16_ENCAPSULATION_OVERHEAD 4
/// Number of bytes used by an S0 encapsulation message
#define S0_ENCAPSULATION_OVERHEAD 20
/// Number of bytes used by an S2 encapsulation message (without SPAN extension)
#define S2_ENCAPSULATION_OVERHEAD 12
/// Number of bytes used by a Multi channel encapsulation
#define MULTI_CHANNEL_ENCAPSULATION_OVERHEAD 4

// Read the granted keys for a node from the attribute store
void zwave_tx_scheme_get_node_granted_keys(zwave_node_id_t node_id,
                                           zwave_keyset_t *keys)
{
  // Default return value:
  *keys = 0;

  // Find out the UNID of the node
  unid_t unid;
  zwave_unid_from_node_id(node_id, unid);

  // Find the Attribute Store entry representing our HomeID:
  // We don't get the node_id using the helper function,
  // as it may not exist and we do not want to create it.
  attribute_store_node_t home_id_node
    = attribute_store_network_helper_get_home_id_node(unid);

  attribute_store_node_t node_id_node
    = attribute_store_get_node_child_by_value(home_id_node,
                                              ATTRIBUTE_NODE_ID,
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&node_id,
                                              sizeof(node_id),
                                              0);

  // Find the granted keys under the NodeID
  attribute_store_node_t granted_keys_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                             0);

  if (granted_keys_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return;
  }

  // Get the attribute store reported value for granted keys.
  attribute_store_read_value(granted_keys_node,
                             REPORTED_ATTRIBUTE,
                             keys,
                             sizeof(zwave_keyset_t));
}

void zwave_tx_scheme_get_zpc_granted_keys(zwave_keyset_t *keys)
{
  // If something goes wrong or nothing is written in the attribute store,
  // We assume to S2 Access Control.
  *keys = ZWAVE_CONTROLLER_S2_ACCESS_KEY;

  // Find the Attribute Store entry representing our NodeID:
  attribute_store_node_t zpc_node = get_zpc_node_id_node();

  attribute_store_node_t zpc_granted_keys_node
    = attribute_store_get_node_child_by_type(zpc_node,
                                             ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                             0);

  // Read from the reported attribute value
  attribute_store_get_reported(zpc_granted_keys_node,
                               keys,
                               sizeof(zwave_keyset_t));
}

void zwave_tx_scheme_get_node_connection_info(
  zwave_node_id_t node_id,
  uint8_t endpoint_id,
  zwave_controller_connection_info_t *connection_info)
{
  connection_info->remote.is_multicast = 0;
  connection_info->local.is_multicast  = 0;
  connection_info->remote.node_id      = node_id;
  connection_info->remote.endpoint_id  = endpoint_id;
  connection_info->local.node_id
    = 0;  // 0 will make the tx component to use ZPC NodeID
  connection_info->local.endpoint_id = 0;

  // Find out the highest granted key of the remote node
  zwave_keyset_t remote_node_keys;
  zwave_tx_scheme_get_node_granted_keys(connection_info->remote.node_id,
                                        &remote_node_keys);

  connection_info->encapsulation
    = zwave_controller_get_highest_encapsulation(remote_node_keys);
}

void zwave_tx_scheme_get_node_tx_options(uint32_t expected_qos_priority,
                                         uint8_t expected_number_of_response,
                                         uint32_t expected_discard_timeout_ms,
                                         zwave_tx_options_t *tx_options)
{
  // Prepare the Z-Wave TX options.
  tx_options->number_of_responses      = expected_number_of_response;
  tx_options->valid_parent_session_id  = false;
  tx_options->use_parent_frame_options = false;
  tx_options->fasttrack                = false;
  tx_options->discard_timeout_ms       = expected_discard_timeout_ms;
  tx_options->qos_priority             = expected_qos_priority;
  tx_options->is_test_frame            = false;
  tx_options->rf_power                 = NORMAL_POWER;
  tx_options->group_id                 = ZWAVE_TX_INVALID_GROUP;
  tx_options->send_follow_ups          = false;
  tx_options->is_first_follow_up       = false;
}

zwave_controller_encapsulation_scheme_t
  zwave_tx_scheme_get_node_highest_security_class(zwave_node_id_t node_id)
{
  zwave_keyset_t remote_node_keys;
  zwave_tx_scheme_get_node_granted_keys(node_id, &remote_node_keys);
  return zwave_controller_get_highest_encapsulation(remote_node_keys);
}

uint16_t zwave_tx_scheme_get_max_payload(zwave_node_id_t node_id,
                                         zwave_endpoint_id_t endpoint_id)
{
  // Gather all the data we need from the attribute store.
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);
  attribute_store_node_t protocol_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                                             0);
  zwave_protocol_t protocol = PROTOCOL_UNKNOWN;
  attribute_store_get_reported(protocol_node, &protocol, sizeof(protocol));

  zwave_controller_encapsulation_scheme_t encapsulation
    = zwave_tx_scheme_get_node_highest_security_class(node_id);

  // Now we are set. First off, which protocol?
  uint16_t maximum_fragment_size = 0;

  if (protocol == PROTOCOL_ZWAVE_LONG_RANGE) {
    maximum_fragment_size = MAX_MSDU_SIZE_LONG_RANGE;
    maximum_fragment_size -= MAX_ROUTING_HEADER_SIZE_LONG_RANGE;

  } else if (protocol == PROTOCOL_ZWAVE) {
    zwave_rf_region_t region = zwapi_get_rf_region();
    if (region == ZWAVE_RF_REGION_KR || region == ZWAVE_RF_REGION_JP) {
      // Regions with R3 only:
      maximum_fragment_size = MAX_MSDU_SIZE_R3;
      maximum_fragment_size -= MAX_ROUTING_HEADER_SIZE_R3;
    } else {
      // Else we have to assume R1/R2:
      maximum_fragment_size = MAX_MSDU_SIZE_R1_R2;
      maximum_fragment_size -= MAX_ROUTING_HEADER_SIZE_R1_R2;
    }
  } else {
    maximum_fragment_size = ZWAVE_TX_SAFE_LOWEST_MAX_PAYLOAD;
    sl_log_warning(LOG_TAG,
                   "Unkonwn protocol for NodeID %d. Selecting the "
                   "lowest possible payload size(%d).",
                   node_id,
                   maximum_fragment_size);
    return maximum_fragment_size;
  }

  // Security encapsulation overhead
  if (encapsulation == ZWAVE_CONTROLLER_ENCAPSULATION_NONE) {
    // Assume that we may decide to use CRC-16 one day.
    maximum_fragment_size -= CRC16_ENCAPSULATION_OVERHEAD;

  } else if (encapsulation == ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0) {
    maximum_fragment_size -= S0_ENCAPSULATION_OVERHEAD;

  } else if (
    (encapsulation == ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS)
    || (encapsulation
        == ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED)
    || (encapsulation
        == ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED)) {
    maximum_fragment_size -= S2_ENCAPSULATION_OVERHEAD;
  }

  // Multi Channel encapsulation
  if (endpoint_id > 0) {
    maximum_fragment_size -= MULTI_CHANNEL_ENCAPSULATION_OVERHEAD;
  }

  return maximum_fragment_size;
}