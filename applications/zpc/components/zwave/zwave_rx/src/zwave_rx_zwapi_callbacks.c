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
// Generic includes
#include <stdlib.h>
#include <stdbool.h>

// Includes from this component
#include "zwave_rx.h"
#include "zwave_rx_internals.h"
#include "zwave_rx_zwapi_callbacks.h"
#include "zwave_controller_internal.h"
#include "zwave_controller_utils.h"
#include "zwave_rx_process.h"

// Includes from other components
#include "zwapi_protocol_controller.h"
#include "zwapi_protocol_mem.h"
#include "zwave_api_transport.h"
#include "sl_log.h"

#define LOG_TAG "zwave_rx_zwapi_callbacks"

// Static helper function
static void
  zwave_rx_map_node_information(zwave_node_id_t node_id,
                                const uint8_t *zwapi_zwave_nif,
                                uint8_t zwapi_zwave_nif_length,
                                zwave_node_info_t *destination_node_info);

void zwave_rx_application_command_handler(uint8_t rx_status,
                                          zwave_node_id_t destination_node_id,
                                          zwave_node_id_t source_node_id,
                                          const uint8_t *zwave_command_payload,
                                          uint8_t zwave_command_payload_length,
                                          int8_t rssi_value)
{
  // We have received a frame!

  // Create a connection info structure with default parameters
  zwave_controller_connection_info_t connection_info = {{}};
  connection_info.local.node_id                      = destination_node_id;
  connection_info.remote.node_id                     = source_node_id;
  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  // Adjust the multicast flag from based on the rx_status bitmask
  if ((rx_status & RECEIVE_STATUS_TYPE_BROAD)
      || (rx_status & RECEIVE_STATUS_TYPE_MULTI)) {
    connection_info.local.is_multicast = true;
  }

  zwave_rx_receive_options_t rx_options = {.status_flags       = rx_status,
                                           .rssi               = rssi_value,
                                           .nodes_in_multicast = {0}};

  // Tell the controller about the Rx event
  zwave_controller_on_frame_reception(source_node_id);

  // Pass the frame on to the Z-Wave controller module
  zwave_controller_on_frame_received(&connection_info,
                                     &rx_options,
                                     zwave_command_payload,
                                     (uint16_t)zwave_command_payload_length);
}

void zwave_rx_application_command_handler_bridge(
  uint8_t rx_status,
  zwave_node_id_t destination_node_id,
  zwave_node_id_t source_node_id,
  const uint8_t *zwave_command_payload,
  uint8_t zwave_command_payload_length,
  int8_t rssi_value)
{
  // Here we treat the incoming frames just like
  // for the other (non-bridge) controller libraries
  zwave_rx_application_command_handler(rx_status,
                                       destination_node_id,
                                       source_node_id,
                                       zwave_command_payload,
                                       zwave_command_payload_length,
                                       rssi_value);
}

void zwave_rx_application_controller_update(uint8_t status,
                                            zwave_node_id_t node_id,
                                            const uint8_t *zwave_nif,
                                            uint8_t zwave_nif_length,
                                            zwave_home_id_t nwi_home_id)
{
  zwave_node_info_t received_nif = {0};
  bool already_included          = false;
  switch (status) {
    case UPDATE_STATE_NEW_ID_ASSIGNED:  // New node in the network
      // Notify the Z-Wave controller, which will dispatch the event further.
      zwave_controller_on_node_id_assigned(node_id, PROTOCOL_ZWAVE);
      break;

    case UPDATE_STATE_DELETE_DONE:  // Node deleted from the network
      // Notify the Z-Wave controller, which will dispatch the event further.
      zwave_controller_on_node_deleted(node_id);
      break;
    case UPDATE_STATE_NODE_INFO_REQ_FAILED:
      zwave_controller_on_node_info_req_failed(node_id);
      break;

    case UPDATE_STATE_NODE_INFO_RECEIVED:  // NIF has been received
      // NIF means we received a frame from that node.
      zwave_controller_on_frame_reception(node_id);
      // Assemble the NIF data first
      zwave_rx_map_node_information(node_id,
                                    zwave_nif,
                                    zwave_nif_length,
                                    &received_nif);

      // Pass on the data to the Z-Wave controller.
      zwave_controller_on_node_information(node_id, &received_nif);
      break;

    case UPDATE_STATE_SUC_ID:  // New SUC NodeID
      // Notify the Z-Wave controller, which will dispatch the event further.
      zwave_controller_on_new_suc(node_id);
      break;

    case UPDATE_STATE_NODE_INFO_FOREIGN_HOMEID_RECEIVED:  // SmartStart Prime received
    case UPDATE_STATE_INCLUDED_NODE_INFO_RECEIVED:  // SmartStart INIF received
    case UPDATE_STATE_NODE_INFO_SMARTSTART_HOMEID_RECEIVED_LR:  // SmartStart Prime received on Long Range
      if (status == UPDATE_STATE_INCLUDED_NODE_INFO_RECEIVED) {
        already_included = true;
      }
      // We are in add mode and received a NIF from another network.
      // It could be NIFs for regular inclusion requets, SmartStart Inclusion
      // requests or INIFs
      // HomeID comes with host endian ordering (from the Z-Wave module)
      zwave_rx_map_node_information(0,
                                    zwave_nif,
                                    zwave_nif_length,
                                    &received_nif);

      // Notify the Z-Wave controller, which will dispatch the event further.
      zwave_controller_on_smart_start_inclusion_request(
        nwi_home_id,
        already_included,
        &received_nif,
        status == UPDATE_STATE_NODE_INFO_SMARTSTART_HOMEID_RECEIVED_LR);
      break;

    default:
      break;
  }
}

void zwave_rx_zwave_api_started(const uint8_t *buffer, uint8_t buffer_length)
{
  (void)buffer;
  (void)buffer_length;
  zwave_api_transport_reset();
  // Make sure we are still running the right settings:
  zwapi_set_node_id_basetype(NODEID_16BITS);

  // Ensure that no add/remove/learn mode is active
  zwapi_add_node_to_network(ADD_NODE_STOP, NULL);
  zwapi_remove_node_from_network(REMOVE_NODE_STOP, NULL);
  zwapi_set_learn_mode(LEARN_MODE_DISABLE, NULL);

  // I can't think of a reason to do something with our NIF here. (buffer, buffer_length)
  zwave_network_management_state_t nms_state = zwave_network_management_get_state();
  if (nms_state != NM_IDLE) {
    sl_log_warning(LOG_TAG,
                "Network management is busy.\n");
    zwave_network_management_abort();
  }
}

/**
 * @brief function that converts the data received from the @ref zwave_api
 *        to fit in a @ref zwave_node_info_t.
 *
 * @param node_id                 The NodeID for which the NIF is being
 *                                forwarded. In case of SmartStart frames,
 *                                NIFs come from foreign netowrks and
 *                                this parameter must be set to 0
 * @param zwapi_zwave_nif         The node info frame data received from
 *                                @ref zwave_api
 * @param zwapi_zwave_nif_length  The length of the data contained in the
 *                                zwapi_zwave_nif pointer
 * @param destination_node_info   A pointer to a zwave_node_info_t where
 *                                the converted data will be written
 */
static void
  zwave_rx_map_node_information(zwave_node_id_t node_id,
                                const uint8_t *zwapi_zwave_nif,
                                uint8_t zwapi_zwave_nif_length,
                                zwave_node_info_t *destination_node_info)
{
  if (zwapi_zwave_nif == NULL || destination_node_info == NULL
      || zwapi_zwave_nif_length == 0) {
    return;
  }

  // First we need to ask extra data to the Z-Wave API before we can proceed
  // (because the NIF received callback does not give us all the NIF fields...)
  if (node_id > 0) {
    zwapi_node_info_header_t zwave_rx_node_info_header = {0};
    sl_status_t command_status
      = zwapi_get_protocol_info(node_id, &zwave_rx_node_info_header);
    if (command_status == SL_STATUS_OK) {
      // Copy our retrieved data into the NIF, else it will just be 0.
      destination_node_info->listening_protocol
        = zwave_rx_node_info_header.capability;
      destination_node_info->optional_protocol
        = zwave_rx_node_info_header.security;
    }
  }
  // Rely on the zwapi_zwave_nif data as much as possible in case
  // command_status was not SL_STATUS_OK
  uint8_t index                             = 0;
  destination_node_info->basic_device_class = zwapi_zwave_nif[index++];
  ZWAVE_RX_VERIFY_OVERFLOW(index, zwapi_zwave_nif_length);

  destination_node_info->generic_device_class = zwapi_zwave_nif[index++];
  ZWAVE_RX_VERIFY_OVERFLOW(index, zwapi_zwave_nif_length);

  destination_node_info->specific_device_class = zwapi_zwave_nif[index++];
  ZWAVE_RX_VERIFY_OVERFLOW(index, zwapi_zwave_nif_length);

  zwave_command_class_list_unpack(destination_node_info,
                                  &zwapi_zwave_nif[index],
                                  zwapi_zwave_nif_length - 3);
}

void zwave_rx_poll_request()
{
  process_poll(&zwave_rx_process);
}
