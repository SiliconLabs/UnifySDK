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
 * @defgroup zpc_attribute_resolver_callbacks ZPC Attribute Resolver callbacks
 * @ingroup zpc_attribute_resolver_internals
 * @brief Collects send data callbacks from @ref attribute_resolver_send and
 * @ref attribute_resolver_group_send and relays them to the
 * @ref attribute_resolver
 *
 * @{
 */

#ifndef ZPC_ATTRIBUTE_RESOLVER_CALLBACKS_H
#define ZPC_ATTRIBUTE_RESOLVER_CALLBACKS_H

#include "attribute_store.h"
#include "attribute_resolver_rule.h"
#include "zwapi_protocol_transport.h"
#include "zwave_tx.h"

/**
 * @brief Struct keeping track of a node
 * resolved as well as the rule type being executed
 */
typedef struct resolution_map {
  attribute_store_node_t node;
  resolver_rule_type_t rule_type;
} resolution_map_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the resources to keep track of ongoing resolutions and
 * process send_data callbacks.
 */
void attribute_resolver_callbacks_reset();

/**
 * @brief Send_data callback for transmissions that did not use Supervision
 * encapsulation.
 *
 * @param status  Indicates how the transmission operation was completed.
 *                Refer for @ref zwapi_transmit_complete_codes for details.
 * @param tx_info zwapi_tx_report_t reported by the @ref zwave_api. It
 *                contains transmission details, refer to @ref zwapi_tx_report_t.
 * @param user    User pointer provided in from this component.
 *                Must be a resolution_map_t pointer
 */
void on_resolver_zwave_send_data_complete(uint8_t status,
                                          const zwapi_tx_report_t *tx_info,
                                          void *user);

/**
 * @brief Send_data callback for transmissions that used Supervision
 * encapsulation.
 *
 * @param supervision_status  Supervision status returned by the node.
 * @param tx_info             zwapi_tx_report_t reported by the @ref zwave_api. It
 *                            contains transmission details, refer to
 *                            \ref zwapi_tx_report_t.
 * @param user                User pointer provided in from this component.
 *                            Must be a resolution_map_t pointer
 */
void on_resolver_zwave_supervision_complete(uint8_t supervision_status,
                                            const zwapi_tx_report_t *tx_info,
                                            void *user);

/**
 * @brief Functions that adds a new node in the resolution list.
 *
 * @param new_node      Attribute Store node being resolved, to add in the list
 * @param rule_type     Rule type being executed.
 *
 */
void add_node_in_resolution_list(attribute_store_node_t new_node,
                                 resolver_rule_type_t rule_type);

/**
 * @brief Helper function verifying if we are currently waiting for a send data
 * callback for a given node resolution.
 *
 * @param node      Attribute Store node being resolved.
 *
 * @returns true if the node is being resolved, false otherwise
 */
bool is_node_in_resolution_list(attribute_store_node_t node);

/**
 * @brief Functions that removes a node from the resolution list.
 *
 * @param node      Attribute Store node no longer being resolved.
 */
void remove_node_from_resolution_list(attribute_store_node_t node);

/**
 * @brief Aborts that removes a node from the resolution list.
 *
 * @param node      Attribute Store node no longer being resolved.
 */
sl_status_t
  attribute_resolver_abort_pending_resolution(attribute_store_node_t node);

/**
 * @brief Links a TX Session ID with a node under resolution.
 *
 * @param node            Attribute Store node being resolved.
 * @param tx_session_id   TX session ID returned by Z-Wave Tx to send a resolving frame.
 *
 * @returns SL_STATUS_OK if associated, SL_STATUS_NOT_FOUND if the node is not under resolution
 */
sl_status_t attribute_resolver_associate_node_with_tx_sessions_id(
  attribute_store_node_t node, zwave_tx_session_id_t tx_session_id);

#ifdef __cplusplus
}
#endif

#endif  //ZPC_ATTRIBUTE_RESOLVER_CALLBACKS_H
/** @} end zpc_attribute_resolver_callbacks */
