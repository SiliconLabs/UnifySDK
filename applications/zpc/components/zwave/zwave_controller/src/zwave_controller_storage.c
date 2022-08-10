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
#include "zwave_controller_storage.h"
#include "sl_log.h"

#include <assert.h>

#define LOG_TAG "zwave_controller_storage"

/**
 * @brief Callback to set a node as S2 capable, meaning that
 *        we know it supports S2,and it will forever do.
 */
typedef sl_status_t (*set_node_as_s2_capable_t)(zwave_node_id_t node_id);
/**
 * @brief Callback to check if a node is supporting S2.
 */
typedef bool (*is_node_S2_capable_t)(zwave_node_id_t node_id);
/**
 * @brief Callback to get granted keys  from persitant storage
 */
typedef sl_status_t (*get_node_granted_keys_t)(zwave_node_id_t node_id,
                                               zwave_keyset_t *keys);
/**
 * @brief Callback to get is a node is included using Z-Wave longrange
 */
typedef zwave_protocol_t (*get_inclusion_protocol_t)(zwave_node_id_t node_id);
/**
 * @brief Callback to get is a the command calss version supported via the node.
 */
typedef zwave_cc_version_t (*zwave_controller_storage_cc_version_t)(
  zwave_command_class_t command_class,
  zwave_node_id_t node_id,
  zwave_endpoint_id_t endpoint_id);

static set_node_as_s2_capable_t set_node_as_s2_capable_cb;
static is_node_S2_capable_t verify_node_is_s2_cb;
static zwave_controller_storage_cc_version_t zwave_node_cc_version_cb;
static get_node_granted_keys_t granted_keys_cb;
static get_inclusion_protocol_t get_inclusion_protocol_cb;

sl_status_t zwave_controller_storage_callback_register(
  const zwave_controller_storage_callback_t *callbacks)
{
  if (callbacks == NULL) {
    sl_log_warning(LOG_TAG,
                   "zwave_controller_storage callbacks are not registered.");
    return SL_STATUS_FAIL;
  }

  set_node_as_s2_capable_cb = callbacks->set_node_as_s2_capable;
  verify_node_is_s2_cb      = callbacks->is_node_S2_capable;
  zwave_node_cc_version_cb  = callbacks->zwave_controller_storage_cc_version;
  granted_keys_cb           = callbacks->get_node_granted_keys;
  get_inclusion_protocol_cb = callbacks->get_inclusion_protocol;

  return SL_STATUS_OK;
}

bool zwave_controller_storage_is_node_s2_capable(zwave_node_id_t node_id)
{
  if (verify_node_is_s2_cb == NULL) {
    sl_log_warning(
      LOG_TAG,
      "Callback that checks if a node is supporting S2 is not registered.");
    return true;
  }
  return (verify_node_is_s2_cb(node_id));
}

sl_status_t
  zwave_controller_storage_as_set_node_s2_capable(zwave_node_id_t node_id)
{
  if (set_node_as_s2_capable_cb == NULL) {
    sl_log_warning(
      LOG_TAG,
      "Callback that sets a node as S2 capable is not registered.");
    return SL_STATUS_FAIL;
  }
  return (set_node_as_s2_capable_cb(node_id));
}

zwave_cc_version_t
  zwave_controller_storage_get_cc_version(zwave_command_class_t command_class,
                                          zwave_node_id_t node_id,
                                          zwave_endpoint_id_t endpoint_id)
{
  if (zwave_node_cc_version_cb == NULL) {
    sl_log_warning(
      LOG_TAG,
      "Callback that helps to get the node cc version is not registered.");
    return 1;
  }
  return zwave_node_cc_version_cb(command_class, node_id, endpoint_id);
}

sl_status_t
  zwave_controller_storage_get_node_granted_keys(zwave_node_id_t node_id,
                                                 zwave_keyset_t *keys)
{
  if (granted_keys_cb == NULL) {
    sl_log_warning(
      LOG_TAG,
      "Callback that helps to get granted keys is not registered.");
    return SL_STATUS_FAIL;
  }
  return granted_keys_cb(node_id, keys);
}

zwave_protocol_t
  zwave_controller_storage_inclusion_protocol(zwave_node_id_t node_id)
{
  if (get_inclusion_protocol_cb != NULL) {
    return get_inclusion_protocol_cb(node_id);
  }
  return PROTOCOL_UNKNOWN;
}
