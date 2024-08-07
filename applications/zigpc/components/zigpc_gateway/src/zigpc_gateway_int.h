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

#ifndef ZIGPC_GATEWAY_INT_H
#define ZIGPC_GATEWAY_INT_H

// ZigPC includes
#include <zigpc_common_zigbee.h>
#include <zigpc_config.h>

/* ZigbeeHost includes */
#include <zigbee_host.h>

#include "zigpc_gateway_notify.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char LOG_TAG[] = "zigpc_gateway";

/* Unique keys to use in datastore */
#define ZIGPC_GATEWAY_NODEID_LIST_KEY  "zb_nodeid_list"
#define ZIGPC_GATEWAY_NODEID_COUNT_KEY "zb_nodeid_count"

/**
 * @brief Container to hold Zigbee device address entries.
 *
 * A Zigbee device can be addressed using the long ID (EUI64) or the short
 * ID (NodeId). This container maps these entries.
 *
 */
typedef struct {
  sl_802154_long_addr_t eui64;
  sl_802154_short_addr_t node_id;
} __attribute__((__packed__)) zigpc_gateway_addr_entry_t;

extern struct zigbeeHostCallbacks zigpc_gateway_zigbee_host_callbacks;

/**
 * Internal API for notify system
 */

/**
 * @brief Reference to Registered Gateway Observers
 *
 */
extern struct zigpc_observable zigpc_gateway_observable;

/**
 * @brief Clear registered observers
 *
 */
sl_status_t zigpc_gateway_reset_observers(void);

/**
 * @brief Z3Gatway callback handler to be called before NCP resets.
 *
 * @param resetStatus   The EZSP reset reason.
 */
void zigpc_gateway_on_ncp_pre_reset(sl_zigbee_ezsp_status_t resetStatus);

/**
 * @brief ZigbeeHost callback handler called after NCP resets and is going
 * through initialization.
 */
void zigpc_gateway_on_ncp_post_reset(void);

/**
 * @brief Load EUI64 to NodeId device ID mappings.
 * This function loads all perisisted EUI64 to NodeId mappings (retrieved from
 * ZigPC datastore) to-be tracked in EmberAf address-table plugin.
 *
 * @return sl_status_t SL_STATUS_OK on successfully loading each mapping,
 * error in reading/loading otherwise.
 */
sl_status_t zigpc_gateway_load_address_table_info(void);

/**
 * @brief Persist EUI64 to NodeId device ID mappings.
 * This function stores all EUI64 to NodeId mappings from the EmberAf
 * address-table plugin into the ZigPC datstore.
 *
 * @return sl_status_t SL_STATUS_OK on successfully persisting each mapping,
 * error in getting/persisting otherwise.
 */
sl_status_t zigpc_gateway_persist_address_table_info(void);

/**
 * @brief Wrapper function to load information from uic_datastore
 * 
 * Plugin information loaded:
 * - emberAf Address Table
 *
 * @return sl_status_t SL_STATUS_OK on success, error otherwise.
 */
sl_status_t zigpc_gateway_load_from_datastore(void);

/**
 * @brief Wrapper function to store information to uic_datastore
 * 
 * Plugin information loaded:
 * - emberAf Address Table
 *
 * @return sl_status_t SL_STATUS_OK on success, error otherwise.
 */
sl_status_t zigpc_gateway_persist_to_datastore(void);

/**
 * @brief zigpc_gateway_unload_sleepy_messages
 * A helper function to unload all ZCL frames stored for a given
 * sleepy device, determined by its eui64
 *
 * @param eui64 - the eui64 of a given sleepy end device
 *
 * @return SL_STATUS_OK if able to successfully find and send the
 * ZCL frames associated with the eui64. SL_STATUS_FAIL otherwise
**/
sl_status_t zigpc_gateway_unload_sleepy_messages(const zigbee_eui64_t eui64);
#ifdef __cplusplus
}
#endif

#endif  // ZIGPC_GATEWAY_INT_H
