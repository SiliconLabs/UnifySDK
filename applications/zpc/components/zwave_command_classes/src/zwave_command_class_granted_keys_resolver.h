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
 * @defgroup zwave_granted_keys Z-Wave Granted Keys Resolver
 * @ingroup command_classes
 * @brief  This component orchestrate the discovery of granted security keys
 * for a NodeID.
 *
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_GRANTED_KEYS_RESOLVER_H
#define ZWAVE_COMMAND_CLASS_GRANTED_KEYS_RESOLVER_H

#include "attribute_store.h"

// Definitions
#include "zwave_keyset_definitions.h"
#include "zwave_controller_connection_info.h"
#include "zwave_node_id_definitions.h"
#include "zwave_generic_types.h"
#include "sl_status.h"

typedef struct zwave_key_protocol_combination {
  zwave_keyset_t key;
  zwave_protocol_t protocol;
} zwave_key_protocol_combination_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Updates the Attribute Store to indicate that the Security key and
 * Protocol that we just used are working with this node.
 *
 * @param node_id_node    Attribute Store node for the NodeID.
 * @param encapsulation   Encapsulation that was used successfully with the node
 *
 */
void zwave_command_class_mark_key_protocol_as_supported(
  attribute_store_node_t node_id_node,
  zwave_controller_encapsulation_scheme_t encapsulation);

/**
 * @brief Registers callbacks to the attribute store and rules
 * to the attribute resolver to be able to resolve Node's capabilities
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_granted_keys_resolver_init(void);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_GRANTED_KEYS_RESOLVER_H
/** @} end zwave_command_class_granted_keys_resolver */
