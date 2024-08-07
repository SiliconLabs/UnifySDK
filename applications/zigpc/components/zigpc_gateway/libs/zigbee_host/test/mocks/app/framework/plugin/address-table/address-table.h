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

#ifndef SILABS_PLUGIN_ADDRESS_TABLE_INTERNAL_H
#define SILABS_PLUGIN_ADDRESS_TABLE_INTERNAL_H

/**
 * This file contains the prototypes and declartions needed to run zigbeeHost unit
 * tests. For more details, refer to:
 * <GSDK>/protocol/zigbee/app/framework/plugin/address-table/address-table.h
 */

#include <stdbool.h>
#define SIGNED_ENUM
#include "stack/include/sl_zigbee_types.h"
#include "app/framework/include/af-types.h"

/*
 * @brief Searches for an EUI64 in the address table. If an entry with a
 * matching EUI64 is found, the function returns the corresponding index.
 * Otherwise the value SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX is returned.
 */
#define sl_zigbee_af_address_table_lookup_by_eui64(longId) \
  sl_zigbee_af_lookup_address_table_entry_by_eui64((longId))
uint8_t sl_zigbee_af_lookup_address_table_entry_by_eui64(sl_802154_long_addr_t longId);

/*
 * @brief If the passed index is valid it copies the entry stored at the passed
 * index to the passed variable and returns SL_STATUS_OK . Otherwise the status
 * SL_STATUS_INVALID_STATE is returned.
 */
sl_status_t emberAfPluginAddressTableLookupByIndex(uint8_t index,
                                                   sl_802154_long_addr_t entry);

/*
 * @brief If the passed index is valid it copies the entry stored at the passed
 * index to the passed variable and returns SL_STATUS_OK . Otherwise the status
 * SL_STATUS_INVALID_STATE is returned.
 */
sl_802154_short_addr_t sl_zigbee_af_address_table_lookup_node_id_by_index(uint8_t index);

/**
 * @brief Allows to send a unicast message specifying the EUI64 of the
 * destination node. Note: the specified EUI64 must be present in the address
 * table, otherwise the status SL_STATUS_INVALID_STATE is returned.
 */
sl_status_t sl_zigbee_af_send_unicast_to_eui64(sl_802154_long_addr_t destination,
                                      sl_zigbee_aps_frame_t *apsFrame,
                                      uint16_t messageLength,
                                      uint8_t *message);

/**
 * @brief Sends the command prepared with emberAfFill.... macro.
 *
 * This function is used to send a command that was previously prepared
 * using the emberAfFill... macros from the client command API.
 * It will be sent as unicast to the node that corresponds in the address table
 * to the passed EUI64.
 */
sl_status_t sl_zigbee_af_send_command_unicast_to_eui64(sl_802154_long_addr_t destination);

/**
 * @brief If the sender EUI64 is available, it is copied to the address parameter
 * and returns SL_STATUS_OK . Otherwise, it returns SL_STATUS_INVALID_STATE .
 */
sl_status_t sl_zigbee_af_get_current_sender_eui64(sl_802154_long_addr_t address);

#endif /* SILABS_PLUGIN_ADDRESS_TABLE_INTERNAL_H */
