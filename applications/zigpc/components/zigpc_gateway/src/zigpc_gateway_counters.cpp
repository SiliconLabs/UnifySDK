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

#include "zigpc_gateway.h"
#include "sl_status.h"

#include "zigpc_gateway_int.h"
#include <unordered_map>

#define COUNTER_NAME_CHAR_BUFFER_LENGTH 50

#define INIT_MAP_COUNTER_ITEM(x) \
  {                              \
    EMBER_COUNTER_##x, #x        \
  }

static std::unordered_map<size_t, const char *> counter_labels
  = {INIT_MAP_COUNTER_ITEM(MAC_RX_BROADCAST),
     INIT_MAP_COUNTER_ITEM(MAC_TX_BROADCAST),
     INIT_MAP_COUNTER_ITEM(MAC_RX_UNICAST),
     INIT_MAP_COUNTER_ITEM(MAC_TX_UNICAST_SUCCESS),
     INIT_MAP_COUNTER_ITEM(MAC_TX_UNICAST_RETRY),
     INIT_MAP_COUNTER_ITEM(MAC_TX_UNICAST_FAILED),
     INIT_MAP_COUNTER_ITEM(APS_DATA_RX_BROADCAST),
     INIT_MAP_COUNTER_ITEM(APS_DATA_TX_BROADCAST),
     INIT_MAP_COUNTER_ITEM(APS_DATA_RX_UNICAST),
     INIT_MAP_COUNTER_ITEM(APS_DATA_TX_UNICAST_SUCCESS),
     INIT_MAP_COUNTER_ITEM(APS_DATA_TX_UNICAST_RETRY),
     INIT_MAP_COUNTER_ITEM(APS_DATA_TX_UNICAST_FAILED),
     INIT_MAP_COUNTER_ITEM(ROUTE_DISCOVERY_INITIATED),
     INIT_MAP_COUNTER_ITEM(NEIGHBOR_ADDED),
     INIT_MAP_COUNTER_ITEM(NEIGHBOR_REMOVED),
     INIT_MAP_COUNTER_ITEM(NEIGHBOR_STALE),
     INIT_MAP_COUNTER_ITEM(JOIN_INDICATION),
     INIT_MAP_COUNTER_ITEM(CHILD_REMOVED),
     INIT_MAP_COUNTER_ITEM(ASH_OVERFLOW_ERROR),
     INIT_MAP_COUNTER_ITEM(ASH_FRAMING_ERROR),
     INIT_MAP_COUNTER_ITEM(ASH_OVERRUN_ERROR),
     INIT_MAP_COUNTER_ITEM(NWK_FRAME_COUNTER_FAILURE),
     INIT_MAP_COUNTER_ITEM(APS_FRAME_COUNTER_FAILURE),
     INIT_MAP_COUNTER_ITEM(ASH_XOFF),
     INIT_MAP_COUNTER_ITEM(APS_LINK_KEY_NOT_AUTHORIZED),
     INIT_MAP_COUNTER_ITEM(NWK_DECRYPTION_FAILURE),
     INIT_MAP_COUNTER_ITEM(APS_DECRYPTION_FAILURE),
     INIT_MAP_COUNTER_ITEM(ALLOCATE_PACKET_BUFFER_FAILURE),
     INIT_MAP_COUNTER_ITEM(RELAYED_UNICAST),
     INIT_MAP_COUNTER_ITEM(PHY_TO_MAC_QUEUE_LIMIT_REACHED),
     INIT_MAP_COUNTER_ITEM(PACKET_VALIDATE_LIBRARY_DROPPED_COUNT),
     INIT_MAP_COUNTER_ITEM(TYPE_NWK_RETRY_OVERFLOW),
     INIT_MAP_COUNTER_ITEM(PHY_CCA_FAIL_COUNT),
     INIT_MAP_COUNTER_ITEM(BROADCAST_TABLE_FULL),
     INIT_MAP_COUNTER_ITEM(PTA_LO_PRI_REQUESTED),
     INIT_MAP_COUNTER_ITEM(PTA_HI_PRI_REQUESTED),
     INIT_MAP_COUNTER_ITEM(PTA_LO_PRI_DENIED),
     INIT_MAP_COUNTER_ITEM(PTA_HI_PRI_DENIED),
     INIT_MAP_COUNTER_ITEM(PTA_LO_PRI_TX_ABORTED),
     INIT_MAP_COUNTER_ITEM(PTA_HI_PRI_TX_ABORTED),
     INIT_MAP_COUNTER_ITEM(ADDRESS_CONFLICT_SENT),
     INIT_MAP_COUNTER_ITEM(TYPE_COUNT)};

const char *zigpc_gateway_get_counters_entry_label(size_t offset)
{
  auto result = counter_labels.find(offset);
  if (result != counter_labels.end()) {
    return result->second;
  } else {
    return "NOT_FOUND";
  }
}

sl_status_t zigpc_gateway_get_counters_list(uint16_t *buffer, size_t count)
{
  sl_status_t status = SL_STATUS_OK;
  if (count < EMBER_COUNTER_TYPE_COUNT) {
    status = SL_STATUS_WOULD_OVERFLOW;
  }

  if (count >= EMBER_COUNTER_TYPE_COUNT) {
    z3gatewayGetCountersList(buffer);
  }
  return status;
}

void zigpc_gateway_clear_counters()
{
  z3gatewayClearCounters();
}

size_t zigpc_gateway_get_counters_capacity()
{
  return EMBER_COUNTER_TYPE_COUNT;
}