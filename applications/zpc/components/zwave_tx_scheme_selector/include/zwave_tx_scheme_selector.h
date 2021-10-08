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
 * @defgroup zwave_tx_scheme_selector A Z-Wave Transmission Scheme Selectors
 * @ingroup components
 * @brief A Z-Wave frame transmission security scheme selector
 *
 * This module selects the right security scheme when sending a Z-Wave frame.
 * This is done by looking up the security scheme from the granted_keys list stored
 * in the attribute system.
 *
 * @{
 */

#ifndef ZWAVE_TX_SCHEME_SELECTOR_H
#define ZWAVE_TX_SCHEME_SELECTOR_H

/// Worst case scenario for the max payload (S0 + Z-Wave R1/R2 + Multi Channel)
#define ZWAVE_TX_SAFE_LOWEST_MAX_PAYLOAD 22

#ifdef __cplusplus
extern "C" {
#endif

#include "zwave_controller_connection_info.h"
#include "zwave_controller_keyset.h"
#include "zwave_tx.h"

/**
 * @brief Return a list of security keys which the controller can use.
 *
 * If the controller started was the first S2 controller in the network
 * this will be all supported keys. But if the controller was included by
 * another controller fewer keys may have been granted.
 *
 * @param[out] keys Key currently granted
 */
void zwave_tx_scheme_get_zpc_granted_keys(zwave_keyset_t *keys);

/**
 * @brief Return a connection info parameters for sending a Z-Wave frame.
 *
 * @param node_id     A remote NodeID where the frame is sent to
 * @param endpoint_id An Endpoint ID where the frame is sent to.
 *                    The function assumes single endpoint addressing
 *
 * @param [out] connection_info for sending a Z-Wave frame
 */
void zwave_tx_scheme_get_node_connection_info(
  zwave_node_id_t node_id,
  uint8_t endpoint_id,
  zwave_controller_connection_info_t *connection_info);

/**
 * @brief Define a Z-Wave frame transmission options
 *
 * @param expected_qos_priority Priority of transmission frame
 * @param expected_number_of_response A number of the expected responses to a frame before
 *                                    transmitting the next frame
 * @param expected_discard_timeout_ms A maximum period in ms which a given frame shall stay in
 *                                    a Z-Wave transmission queue before it is dropped.
 *
 * @param [out] tx_options transmission frame configuration options
 *
 */
void zwave_tx_scheme_get_node_tx_options(uint32_t expected_qos_priority,
                                         uint8_t expected_number_of_response,
                                         uint32_t expected_discard_timeout_ms,
                                         zwave_tx_options_t *tx_options);

/**
 * @brief Return the granted keys for a node.
 *
 * @param [in] node_id Z-Wave NodeID of the node
 * @param [out] keys Pointer where to write granted keys.
 */
void zwave_tx_scheme_get_node_granted_keys(zwave_node_id_t node_id,
                                           zwave_keyset_t *keys);

/**
 * @brief Return the Highest security Class of a NodeID.
 *
 * @param [in] node_id Z-Wave NodeID of the node
 * @returns Highest security class of the NodeID.
 */
zwave_controller_encapsulation_scheme_t
  zwave_tx_scheme_get_node_highest_security_class(zwave_node_id_t node_id);

/**
 * @brief Computes the maximum number of bytes that can be sent to a given NodeID.
 *        (lowest layer)
 *
 * @param node_id             The NodeID of the destination
 * @returns The number of bytes that can be sent to the destination
 */
uint16_t zwave_tx_scheme_get_max_payload(zwave_node_id_t node_id);

/**
 * @brief Computes how long of a frame can be sent to a given NodeID / Endpoint
 *        when queuing a frame at the application level.
 *
 * @param node_id             The NodeID of the destination
 * @param endpoint_id         The Endpoint ID to send to
 * @returns The number of bytes that can be sent to the destination before
 *          all additional encapsulations such as Security or Multi Channel
 *          encapsulation.
 */
uint16_t
  zwave_tx_scheme_get_max_application_payload(zwave_node_id_t node_id,
                                              zwave_endpoint_id_t endpoint_id);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_TX_SCHEME_SELECTOR_H
/** @} end zwave_tx_scheme_selector */
