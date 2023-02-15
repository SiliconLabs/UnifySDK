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
 * @defgroup zwave_controller_utils Z-Wave Controller Utils
 * @ingroup zwave_controller
 * @brief Various utilities for the Z-Wave Controller
 *
 * @{
 */

#ifndef ZWAVE_CONTROLLER_UTILS_H
#define ZWAVE_CONTROLLER_UTILS_H

// includes from this component
#include "zwave_controller_connection_info.h"

// Includes from other components
#include "zwave_rx_definitions.h"
#include "zwave_tx.h"

// Interfaces
#include "zwave_command_class_version_types.h"

// Generic includes
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Send a Z-Wave NOP frame to Z-Wave NodeID
 *
 * @param node_id             Destination NodeID
 * @param qos_priority        QoS Priority of the NOP frame
 * @param discard_timeout_ms  Discard timeout for the NOP frame
 * @param on_send_complete    Z-Wave Tx on send complete callback
 * @param user                User data pointer
 *
 * @returns status code returned by @ref zwave_tx_send_data
 */
sl_status_t zwave_send_nop_to_node(
  zwave_node_id_t node_id,
  uint32_t qos_priority,
  uint32_t discard_timeout_ms,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user);

/**
 * @brief Log the Z-Wave received frame data using the sl_log system.
 *
 * This function takes the arguments from zwave_rx_callbacks_t
 * on_frame_received callback from the \ref zwave_rx and uses the sl_log to
 * pretty print the data.
 * @param connection_info   Refer to \ref zwave_controller_connection_info_t
 * @param rx_options        Refer to \ref zwave_rx_receive_options_t
 * @param frame_data        Pointer to de-encapsulated data
 * @param frame_length      Length of data contained in the frame_data pointer.
 */
void zwave_sl_log_frame_data(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length);

/**
 * @brief Log the Z-Wave received NIF using the sl_log system.
 *
 * This function takes the arguments from zwave_rx_callbacks_t
 * on_node_information callback from the \ref zwave_rx and uses the sl_log to
 * pretty print the data.
 * @param node_id           NodeID of the node to which the NIF belongs
 * @param node_info         Pointer to a \ref zwave_node_info_t structure
 */
void zwave_sl_log_nif_data(zwave_node_id_t node_id,
                           const zwave_node_info_t *node_info);

/**
 * @brief Prints a DSK to the log, using the log
 *
 * @param tag LOG_TAG to use for \ref sl_log_debug()
 * @param dsk To print out
 */
void zwave_sl_log_dsk(const char *tag, const zwave_dsk_t dsk);

/**
 * @brief Unpack the given NIF and parse it into command class list in
 * zwave_node_info and fill the length of resulted command class list.
 *
 * @param [out] node_info Struct where the parsed result is stored into
 * @param nif NIF as a byte array
 * @param nif_length Length of incoming NIF
 */
void zwave_command_class_list_unpack(zwave_node_info_t *node_info,
                                     const uint8_t *nif,
                                     uint8_t nif_length);

/**
 * @brief Pack the given NIF into a uint8_t array.
 *
 * @param node_info   A pointer to a struct to convert to a byte array.
 * @param nif         A pointer to a byte array where the node_info
 *                    data will be written
 * @param nif_length  A pointer where the length of the written data
 *                    in the NIF buffer will be indicated
 */
void zwave_command_class_list_pack(const zwave_node_info_t *node_info,
                                   uint8_t *nif,
                                   uint8_t *nif_length);

/**
 * @brief Verifies if a CC is in a raw array of Command Classes.
 *
 * @param command_class  The Command Class to search for
 * @param nif            A pointer to a byte array where to look for the CC
 * @param nif_length      The length of the data contained in the NIF array.
 * @returns true if the Command Class is found, false otherwise.
 */
bool is_command_class_in_supported_list(zwave_command_class_t command_class,
                                        const uint8_t *nif,
                                        uint8_t nif_length);

/**
 * @brief Parses a NIF byte array and converts it into a lists of
 * supported/controlled Command Classes
 *
 * The passed arrays (supported_command_classes and controlled_command_classes)
 * MUST be able to contain ZWAVE_MAX_NIF_SIZE elements.
 *
 * @param nif                               The byte array containing Command
 *                                          Classes as received from the Z-Wave
 *                                          Protocol
 * @param nif_length                        The number of bytes to parse from the
 *                                          NIF array.
 * @param supported_command_classes         The array where to write supported
 *                                          command classes found in the NIF
 *                                          array.
 * @param supported_command_classes_length  A pointer where to write the number
 *                                          of elements that were written to the
 *                                          supported_command_classes array
 * @param controlled_command_classes        The array where to write controlled
 *                                          command classes found in the NIF
 *                                          array.
 * @param controlled_command_classes_length A pointer where to write the number
 *                                          of elements that were written to the
 *                                          controlled_command_classes array
 * @returns SL_STATUS_OK if the parsing went well and the data copied in
 * @returns SL_STATUS_FAIL if an error occurred.
 */
sl_status_t zwave_parse_nif(const uint8_t *nif,
                            size_t nif_length,
                            zwave_command_class_t *supported_command_classes,
                            size_t *supported_command_classes_length,
                            zwave_command_class_t *controlled_command_classes,
                            size_t *controlled_command_classes_length);

/**
 * @brief Return a string representation of the Network Scheme
 *
 * @param scheme
 * @return const char*
 */
const char *
  zwave_network_scheme_str(zwave_controller_encapsulation_scheme_t scheme);

/**
 * @brief Get the highest Security Class of the ZPC for the current network
 * @returns zwave_controller_encapsulation_scheme_t
 */
zwave_controller_encapsulation_scheme_t zpc_highest_security_class();

#ifdef __cplusplus
}
#endif

#endif  // ZWAVE_CONTROLLER_UTILS_H

/** @} end zwave_controller_utils */
