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

/* Z3gateway includes */
#include "z3gateway.h"

#include "zigpc_gateway_notify.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char LOG_TAG[] = "zigpc_gateway";

/* Unique keys to use in datastore */
#define ZIGPC_GATEWAY_STORE_DEVICE_TABLE_ARR_KEY  "zigpc_gsdk_dt"
#define ZIGPC_GATEWAY_STORE_DEVICE_TABLE_SIZE_KEY "zigpc_gsdk_dt_size"
#define ZIGPC_GATEWAY_STORE_DEVICE_TABLE_CLUSTER_ARR_KEY \
  "zigpc_gsdk_dt_clusters"
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
  EmberEUI64 eui64;
  EmberNodeId node_id;
} __attribute__((__packed__)) zigpc_gateway_addr_entry_t;

extern struct z3gatewayCallbacks zigpc_gateway_z3gateway_callbacks;

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
void zigpc_gateway_on_ncp_pre_reset(EzspStatus resetStatus);

/**
 * @brief Z3GAteway callback handler called after NCP resets and is going
 * through initialization.
 *
 * @param ncpMemConfigureStage  Flag to indicate NCP configuration stages. TRUE
 * if only memory-configure EZSP messages are accepted at this stage; FALSE if
 * only non-memory-configure EZSP messaged are accepted by the NCP.
 */
void zigpc_gateway_on_ncp_post_reset(bool ncpMemConfigureStage);

/**
 * @brief Retrieve persisted information related to Z3Gateway and ZigPC Gateway
 * from the datastore. This includes:
 * - GSDK device-table plugin state: first the number of entries stored in the
 * datastore is retrieved. Then, each datastore entry (and the associated
 * cluster list) is loaded and imported using the z3gateway device table import
 * entry API.
 *
 * @return sl_status_t SL_STATUS_OK if all information was loaded from datastore
 * successfully, error status if not.
 */
sl_status_t zigpc_gateway_load_device_table_info(void);

/**
 * @brief Persist information related to Z3Gateway and ZigPC Gateway to the
 * datastore. This includes:
 * - GSDK device-table plugin state: first the number of entries used by the
 * device table is retrieved. Then, each entry (and the associated cluster
 * list) is exported using the z3gateway API and saved in the datastore as
 * three separate info types: device_table size, device_table_entry[i], and
 * device_table_entry[i].clusterList.
 *
 * @return sl_status_t SL_STATUS_OK if all information was store to datastore
 * successfully, error status if not.
 */
sl_status_t zigpc_gateway_persist_device_table_info(void);

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
 * @brief Wrapper function to load information to both EmberAf device-table and
 * address-table plugins. Refer to zigpc_gateway_load_device_table_info and
 * zigpc_gateway_load_address_table_info for more information.
 *
 * @return sl_status_t SL_STATUS_OK on success, error otherwise.
 */
sl_status_t zigpc_gateway_load_from_datastore(void);

/**
 * @brief Wrapper function to store information from both EmberAf device-table and
 * address-table plugins into the datastore. Refer to
 * zigpc_gateway_persist_device_table_info and
 * zigpc_gateway_persist_address_table_info for more information.
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
