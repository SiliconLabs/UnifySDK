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
 * @defgroup zwave_command_class_firmware_update_internals Firmware Update Command Class internal types
 * @ingroup zwave_command_class_firmware_update
 * @brief Internal types and definitions for the Firmware Update Command Class
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_FIRMWARE_UPDATE_INTERNALS_H
#define ZWAVE_COMMAND_CLASS_FIRMWARE_UPDATE_INTERNALS_H

#include "attribute_store.h"

/// Firmware transfer state value
typedef enum zwave_firmware_transfer_state {
  // No Firmware transfer is ongoing
  IDLE = 0,
  // A Firmware transfer is ongoing
  ONGOING = 1,
  // A Firmware transfer is completed, wait for a subsequent activation
  WAITING_FOR_ACTIVATION = 2,
  // A Firmware transfer is completed, waiting for the node to reboot
  WAITING_FOR_REBOOT = 3
} zwave_firmware_transfer_state_t;

/// Firmware transfer last status/error
typedef enum zwave_firmware_transfer_status {
  SUCCESS            = 0,
  ABORTED            = 1,
  NOT_AUTHORIZED     = 2,
  INVALID_IMAGE      = 3,
  REQUIRE_MORE_IMAGE = 4,
  NOT_SUPPORTED      = 5,
} zwave_firmware_transfer_status_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sets the Firmware data file for a Firmware Update.
 *
 * It will configure the Firmware transfer to use this file
 *
 * @param firmware_transfer_node  Attribute Store node under which the Firmware
 *                                Target must be set.
 * @param file_name               String that indicates the path of a
 *                                file on the file-system
 *
 * @returns sl_status_t code.
 */
sl_status_t
  set_firmware_transfer_file(attribute_store_node_t firmware_transfer_node,
                             const char *file_name);

/**
 * @brief Sets the Firmware transfer last status for a Firmware Update.
 *
 * @param firmware_transfer_node  Attribute Store node under which the Firmware
 *                                Target must be set.
 * @param status                  zwave_firmware_transfer_status_t to set
 *
 * @returns sl_status_t code.
 */
sl_status_t set_firmware_transfer_last_status(
  attribute_store_node_t firmware_transfer_node,
  zwave_firmware_transfer_status_t status);

/**
 * @brief Sets the current offset for a Firmware Transfer
 *
 * @param firmware_transfer_node  Attribute Store node under which the Firmware
 *                                Target must be set.
 * @param transfer_offset         The value of the current offset
 *
 * @returns sl_status_t code.
 */
sl_status_t set_transfer_offset(attribute_store_node_t firmware_transfer_node,
                                uint32_t transfer_offset);

/**
 * @brief Gets the theoretical max fragment size with a node/endpoint
 *
 * It will use the Security Class, inclusion protocol and RF Region configuration
 * to calculate the maximum amount of data that can be passed in each frame.
 *
 * If the ep>0, we also account for Multi Channel Encapsulation.
 *
 * @param endpoint_node  Attribute Store node for the Endpoint.
 *
 * @returns sl_status_t code.
 */
uint16_t
  get_theoretical_max_fragment_size(attribute_store_node_t endpoint_node);

/**
 * @brief Sets the Firmware transfer fragment size for a Firmware Update.
 *
 * It will find the information from the Firmware Update capabilities and lock
 * it for that firmware transfer.
 *
 * @param firmware_transfer_node  Attribute Store node under which the Firmware
 *                                Target must be set.
 *
 * @returns sl_status_t code.
 */
sl_status_t set_firmware_transfer_fragment_size(
  attribute_store_node_t firmware_transfer_node);

/**
 * @brief Verifies if a node supports Firmware Activation
 *
 * @param endpoint_node  Attribute Store node for the Endpoint ID
 *
 * @returns true if supported, false otherwise
 */
bool node_supports_activation(attribute_store_node_t endpoint_node);

/**
 * @brief Verifies if a Firmware Target value is valid for an endpoint.
 *
 * @param endpoint_node     Attribute Store node for the Endpoint ID
 * @param firmware_target   Firmware target to look for.
 *
 * @returns true if the firmware target is valid, false otherwise
 */
bool is_firmware_target_valid(attribute_store_node_t endpoint_node,
                              uint32_t firmware_target);

/**
 * @brief Verifies if a Firmware Update is ongoing for an endpoint.
 *
 * @param endpoint_node     Attribute Store node for the Endpoint ID
 *
 * @returns true if a firmware update is ongoing, false otherwise
 */
bool is_firmware_update_ongoing(attribute_store_node_t endpoint_node);

/**
 * @brief Postpones the Firmware Update expiry for an Endpoint.
 *
 * @param endpoint_node     Attribute Store node for the Endpoint ID
 *
 * @returns SL_STATUS_OK if it was postponed successfully.
 */
sl_status_t
  postpone_firmware_update_expiry(attribute_store_node_t endpoint_node);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_FIRMWARE_UPDATE_INTERNALS_H
/** @} end zwave_command_class_firmware_update_internals */
