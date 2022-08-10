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
#include "zwave_controller_callbacks.h"
#include "zwave_controller_transport_internal.h"
#include "zwave_controller_internal.h"
#include "zwave_controller_utils.h"
#include "zwapi_protocol_controller.h"

// Includes from other components
#include "zwave_network_management.h"

// Generic includes
#include <stddef.h>

// Unify includes
#include "sl_log.h"
#define LOG_TAG "zwave_controller_dispatch"

// Our variables for the component
const zwave_controller_callbacks_t *zwave_controller_callbacks_array
  [ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS]
  = {NULL};

static zwave_controller_reset_step_t
  zwave_controller_reset_steps[ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS]
  = {NULL};
static bool reset_ongoing;

sl_status_t zwave_controller_register_callbacks(
  const zwave_controller_callbacks_t *callbacks)
{
  if (callbacks == NULL) {
    return SL_STATUS_FAIL;
  }

  // Verify if this user already registered its callback pointer:
  for (uint8_t i = 0; i < ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS; i++) {
    if (zwave_controller_callbacks_array[i] == callbacks) {
      return SL_STATUS_OK;
    }
  }

  // If not registered, look for the first empty spot
  for (uint8_t i = 0; i < ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS; i++) {
    if (zwave_controller_callbacks_array[i] == NULL) {
      zwave_controller_callbacks_array[i] = callbacks;
      return SL_STATUS_OK;
    }
  }

  sl_log_error(LOG_TAG,
               "Cannot register Z-Wave Controller callbacks. "
               "No More callback slots available.");
  return SL_STATUS_NO_MORE_RESOURCE;
}

sl_status_t zwave_controller_deregister_callbacks(
  const zwave_controller_callbacks_t *callbacks)
{
  if (callbacks == NULL) {
    return SL_STATUS_FAIL;
  }
  // Find out the current slot used by the callback
  for (uint8_t i = 0; i < ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS; i++) {
    if (zwave_controller_callbacks_array[i] == callbacks) {
      zwave_controller_callbacks_array[i] = NULL;
      return SL_STATUS_OK;
    }
  }

  // If the callback was not present, we just tell the user
  // that deregistration was successful.
  return SL_STATUS_OK;
}

sl_status_t zwave_controller_register_reset_step(
  zwave_controller_reset_step_t step_function, int32_t priority)
{
  if (step_function == NULL) {
    return SL_STATUS_FAIL;
  }
  if ((priority < ZWAVE_CONTROLLER_RESET_STEP_MIN_PRIORITY)
      || (priority >= ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS)) {
    return SL_STATUS_FAIL;
  }

  if (zwave_controller_reset_steps[priority] != NULL) {
    if (zwave_controller_reset_steps[priority] != step_function) {
      return SL_STATUS_FAIL;
    } else {
      return SL_STATUS_OK;
    }
  }
  zwave_controller_reset_steps[priority] = step_function;
  return SL_STATUS_OK;
}

void zwave_controller_reset()
{
  if (reset_ongoing == false) {
    reset_ongoing = true;
    zwave_controller_on_reset_step_complete(
      ZWAVE_CONTROLLER_RESET_STEP_MIN_PRIORITY - 1);
  }
}

bool zwave_controller_is_reset_ongoing()
{
  return reset_ongoing;
}

void zwave_controller_on_reset_step_complete(int32_t priority)
{
  if (reset_ongoing == false) {
    return;
  }
  for (uint8_t i = priority + 1;
       i < ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS;
       i++) {
    if (zwave_controller_reset_steps[i] != NULL) {
      // Invoke the next in the queue and wait for callback if it returns OK
      sl_status_t step_status = zwave_controller_reset_steps[i]();
      if (step_status == SL_STATUS_OK) {
        return;
      }
    }
  }
  reset_ongoing = false;
}

void zwave_controller_callbacks_init(void)
{
  reset_ongoing = false;
  for (uint8_t i = 0; i < ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS; i++) {
    zwave_controller_callbacks_array[i] = NULL;
    zwave_controller_reset_steps[i]     = NULL;
  }
}

/// Helper macro to call all registered callbacks in the callback array
#define ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(callback_function, ...)          \
  for (uint8_t i = 0; i < ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS;     \
       i++) {                                                                \
    if (zwave_controller_callbacks_array[i] != NULL                          \
        && zwave_controller_callbacks_array[i]->callback_function != NULL) { \
      zwave_controller_callbacks_array[i]->callback_function(__VA_ARGS__);   \
    }                                                                        \
  }

/**************************** dispatches ***********************************/

void zwave_controller_on_state_updated(zwave_network_management_state_t state)
{
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_state_updated, state)
}

void zwave_controller_on_error(zwave_network_management_error_t error)
{
  sl_log_info(LOG_TAG,
              "Network Management error has occurred. Error: %d",
              error);
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_error, error)
}

void zwave_controller_on_node_id_assigned(zwave_node_id_t node_id,
                                          zwave_protocol_t inclusion_protocol)
{
  // Decide if it was included by us based on if the network management is idle
  // or not.
  bool included_by_us = zwave_network_management_is_busy();
  if (included_by_us) {
    sl_log_info(LOG_TAG,
                "New NodeID assigned (%d), "
                "it was assigned by the ZPC",
                node_id);
  } else {
    sl_log_info(LOG_TAG,
                "New NodeID assigned (%d), "
                "it was assigned by another controller.",
                node_id);
  }

  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_node_id_assigned,
                                      node_id,
                                      included_by_us,
                                      inclusion_protocol)
}

void zwave_controller_on_node_deleted(zwave_node_id_t node_id)
{
  if (node_id != 0) {
    sl_log_info(LOG_TAG, "NodeID %d has left the network. ", node_id);
  } else {
    sl_log_info(LOG_TAG, "Excluded a NodeID from another network. ");
  }
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_node_deleted, node_id)
}

void zwave_controller_on_node_info_req_failed(zwave_node_id_t node_id)
{
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_node_info_req_failed, node_id)
}

void zwave_controller_on_frame_transmission(bool transmission_successful,
                                            const zwapi_tx_report_t *tx_status,
                                            zwave_node_id_t node_id)
{
  if (transmission_successful == false) {
    sl_log_info(LOG_TAG, "Transmission failure with NodeID %d", node_id);
  }

  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_frame_transmission,
                                      transmission_successful,
                                      tx_status,
                                      node_id)
}

void zwave_controller_on_node_added(sl_status_t status,
                                    const zwave_node_info_t *nif,
                                    zwave_node_id_t node_id,
                                    zwave_dsk_t dsk,
                                    zwave_keyset_t granted_keys,
                                    zwave_kex_fail_type_t kex_fail_type,
                                    zwave_protocol_t inclusion_protocol)
{
  sl_log_info(LOG_TAG,
              "New NodeID %d has joined the network. "
              "Granted keys: 0x%02X - Protocol: %d",
              node_id,
              granted_keys,
              inclusion_protocol);
  zwave_sl_log_dsk(LOG_TAG, dsk);
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_node_added,
                                      status,
                                      nif,
                                      node_id,
                                      dsk,
                                      granted_keys,
                                      kex_fail_type,
                                      inclusion_protocol)
}

void zwave_controller_on_network_address_update(zwave_home_id_t home_id,
                                                zwave_node_id_t node_id)
{
  sl_log_info(LOG_TAG,
              "Network address update: "
              "HomeID %08X - NodeID: %d",
              home_id,
              node_id);

  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_network_address_update,
                                      home_id,
                                      node_id)
}

void zwave_controller_on_new_network_entered(
  zwave_home_id_t home_id,
  zwave_node_id_t node_id,
  zwave_keyset_t granted_keys,
  zwave_kex_fail_type_t kex_fail_type)
{
  sl_log_info(LOG_TAG,
              "New network ready to be operated. "
              "HomeID %08X - ZPC NodeID: %d - Granted keys: 0x%02X",
              home_id,
              node_id,
              granted_keys);
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_new_network_entered,
                                      home_id,
                                      node_id,
                                      granted_keys,
                                      kex_fail_type)
}

void zwave_controller_on_keys_report(bool csa, zwave_keyset_t keys)
{
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_keys_report, csa, keys);
}

void zwave_controller_on_dsk_report(uint8_t input_length,
                                    zwave_dsk_t dsk,
                                    zwave_keyset_t keys)
{
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_dsk_report, input_length, dsk, keys)
}

void zwave_controller_on_frame_reception(zwave_node_id_t node_id)
{  // Inform listeners that we just had an Rx with a node
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_rx_frame_received, node_id)
}

void zwave_controller_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  sl_status_t status
    = zwave_controller_transport_on_frame_received(connection_info,
                                                   rx_options,
                                                   frame_data,
                                                   frame_length);
  //If no transport plugins needs the frame parse it on to upper layers
  if (status == SL_STATUS_NOT_FOUND) {
    ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_application_frame_received,
                                        connection_info,
                                        rx_options,
                                        frame_data,
                                        frame_length);
  }
}

void zwave_controller_on_smart_start_inclusion_request(
  uint32_t home_id,
  bool already_included,
  const zwave_node_info_t *node_info,
  zwave_protocol_t inclusion_protocol)
{
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_smart_start_inclusion_request,
                                      home_id,
                                      already_included,
                                      node_info,
                                      inclusion_protocol)
}

void zwave_controller_on_node_information(zwave_node_id_t node_id,
                                          const zwave_node_info_t *node_info)
{
  zwave_sl_log_nif_data(node_id, node_info);
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_node_information, node_id, node_info)
}

void zwave_controller_on_new_suc(zwave_node_id_t suc_node_id)
{
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_new_suc, suc_node_id)
}

void zwave_controller_on_multicast_group_deleted(
  zwave_multicast_group_id_t group_id)
{
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_multicast_group_deleted, group_id)
}

void zwave_controller_on_request_neighbor_update(uint8_t status)
{
  ZWAVE_CONTROLLER_DISPATCH_CALLBACKS(on_request_neighbor_update, status)
}
