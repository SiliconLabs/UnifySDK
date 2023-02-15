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
#include <string.h>
#include <stdio.h>

// Includes from this component
#include "zwapi_internal.h"
#include "zwapi_func_ids.h"
#include "zwapi_protocol_controller.h"  // For LEARN_INFO
#include "zwapi_protocol_transport.h"   // For zwapi_tx_report_t
#include "zwapi_session.h"
#include "zwapi_init.h"
#include "zwapi_utils.h"

// Includes from other components
#include "zwave_controller_endian.h"
#include "sl_log.h"

#define LOG_TAG "zwapi_protocol_rx_dispatch"

static LEARN_INFO learnNodeInfo;

// Our local command buffer
static uint8_t zwapi_command_buffer[REQUEST_BUFFER_SIZE];

/**
  * Serial API response frame callbacks
  */
void (*zwapi_send_data_callback)(uint8_t, zwapi_tx_report_t *);
void (*zwapi_send_nop_callback)(uint8_t, zwapi_tx_report_t *);
void (*zwapi_send_data_multi_callback)(uint8_t);
void (*zwapi_send_test_frame_callback)(uint8_t);
void (*zwapi_send_data_bridge_callback)(uint8_t, zwapi_tx_report_t *);
void (*zwapi_send_data_multi_bridge_callback)(uint8_t);
void (*zwapi_send_node_information_callback)(uint8_t txStatus);
void (*zwapi_memory_put_buffer_callback)(void);
void (*zwapi_set_default_callback)(void);
void (*zwapi_create_new_primarly_ctrl_callback)(struct LEARN_INFO *);
void (*zwapi_remove_node_from_network_callback)(struct LEARN_INFO *);
void (*zwapi_add_node_to_network_callback)(struct LEARN_INFO *);
void (*zwapi_transfer_primary_ctrl_role_callback)(struct LEARN_INFO *);
void (*zwapi_send_controller_replication_callback)(uint8_t txStatus);
void (*zwapi_assign_return_route_callback)(uint8_t bStatus);
void (*zwapi_assign_suc_return_route_callback)(uint8_t bStatus);
void (*zwapi_delete_suc_return_route_callback)(uint8_t bStatus);
void (*zwapi_delete_return_route_callback)(uint8_t bStatus);
void (*zwapi_set_learn_mode_callback)(struct LEARN_INFO *);
void (*zwapi_set_virtual_node_to_learn_mode_callback)(uint8_t bStatus,
                                                      zwave_node_id_t orgID,
                                                      zwave_node_id_t newID);
void (*zwapi_request_neighbor_update_callback)(uint8_t bStatus);
void (*zwapi_set_suc_node_id_callback)(uint8_t bStatus);
void (*zwapi_request_network_update_callback)(uint8_t txStatus);
void (*zwapi_remove_failed_node_callback)(uint8_t txStatus);
void (*zwapi_replace_failed_node_callback)(uint8_t txStatus);

void zwave_api_protocol_init_callbacks(void)
{
  zwapi_send_data_callback                      = NULL;
  zwapi_send_nop_callback                       = NULL;
  zwapi_send_data_multi_callback                = NULL;
  zwapi_send_test_frame_callback                = NULL;
  zwapi_send_data_bridge_callback               = NULL;
  zwapi_send_data_multi_bridge_callback         = NULL;
  zwapi_send_node_information_callback          = NULL;
  zwapi_memory_put_buffer_callback              = NULL;
  zwapi_set_default_callback                    = NULL;
  zwapi_create_new_primarly_ctrl_callback       = NULL;
  zwapi_remove_node_from_network_callback       = NULL;
  zwapi_add_node_to_network_callback            = NULL;
  zwapi_transfer_primary_ctrl_role_callback     = NULL;
  zwapi_send_controller_replication_callback    = NULL;
  zwapi_assign_return_route_callback            = NULL;
  zwapi_assign_suc_return_route_callback        = NULL;
  zwapi_delete_suc_return_route_callback        = NULL;
  zwapi_delete_return_route_callback            = NULL;
  zwapi_set_learn_mode_callback                 = NULL;
  zwapi_set_virtual_node_to_learn_mode_callback = NULL;
  zwapi_request_neighbor_update_callback        = NULL;
  zwapi_set_suc_node_id_callback                = NULL;
  zwapi_request_network_update_callback         = NULL;
  zwapi_remove_failed_node_callback             = NULL;
  zwapi_replace_failed_node_callback            = NULL;
}

/* Check if the received frame will overflow zwapi_command_buffer buffer */
static bool zwave_api_detect_buffer_overflow(uint16_t len)
{
  if (len > sizeof(zwapi_command_buffer)) {
    sl_log_error(LOG_TAG, "Buffer overflow, dropping Z-Wave API frame\n");
    return true;
  }
  return false;
}

static const char *zwapi_frame_to_string(const uint8_t *buffer,
                                         uint16_t buffer_length)
{
  static char message[1000] = {'\0'};
  uint16_t index            = 0;
  for (uint16_t i = 0; i < buffer_length; i++) {
    if (i == IDX_LEN) {
      index += snprintf(message + index, sizeof(message) - index, "Length=");
    } else if (i == IDX_TYPE) {
      index += snprintf(message + index, sizeof(message) - index, "Type=");
    } else if (i == IDX_CMD) {
      index += snprintf(message + index, sizeof(message) - index, "Cmd=");
    }
    index
      += snprintf(message + index, sizeof(message) - index, "%02X ", buffer[i]);
  }
  return message;
}

/**
 * @brief Fills default values in the Tx Report in case it is too short or
 * incomplete coming from the Z-Wave API
 *
 * @param tx_report [out] Pointer to the struct where the default values are written.
 */
static void fill_default_tx_report_values(zwapi_tx_report_t *tx_report)
{
  tx_report->transmit_ticks                            = 0;
  tx_report->number_of_repeaters                       = 0;
  tx_report->ack_rssi                                  = RSSI_NOT_AVAILABLE;
  tx_report->rssi_values.incoming[0]                   = RSSI_NOT_AVAILABLE;
  tx_report->rssi_values.incoming[1]                   = RSSI_NOT_AVAILABLE;
  tx_report->rssi_values.incoming[2]                   = RSSI_NOT_AVAILABLE;
  tx_report->rssi_values.incoming[3]                   = RSSI_NOT_AVAILABLE;
  tx_report->ack_channel_number                        = 0;
  tx_report->tx_channel_number                         = 0;
  tx_report->last_route_repeaters[0]                   = 0;
  tx_report->last_route_repeaters[1]                   = 0;
  tx_report->last_route_repeaters[2]                   = 0;
  tx_report->last_route_repeaters[3]                   = 0;
  tx_report->beam_250ms                                = false;
  tx_report->beam_1000ms                               = false;
  tx_report->last_route_speed                          = UNKNOWN_SPEED;
  tx_report->routing_attempts                          = 0;
  tx_report->last_failed_link.from                     = 0;
  tx_report->last_failed_link.to                       = 0;
  tx_report->tx_power                                  = RSSI_NOT_AVAILABLE;
  tx_report->measured_noise_floor                      = RSSI_NOT_AVAILABLE;
  tx_report->destination_ack_mpdu_tx_power             = RSSI_NOT_AVAILABLE;
  tx_report->destination_ack_mpdu_measured_rssi        = RSSI_NOT_AVAILABLE;
  tx_report->destination_ack_mpdu_measured_noise_floor = RSSI_NOT_AVAILABLE;
}

void zwave_api_protocol_rx_dispatch(uint8_t *pData, uint16_t len)
{
  int i                               = 0;
  void (*funcLearnInfo)(LEARN_INFO *) = 0;
  void (*f)(uint8_t, zwapi_tx_report_t *);
  void (*f2)(uint8_t);

  sl_log_debug(LOG_TAG,
               "Incoming Z-Wave API frame (hex): %s\n",
               zwapi_frame_to_string(pData, len));

  // The zwapi_connection module listens for the responses
  if (pData[IDX_TYPE] == FRAME_TYPE_RESPONSE) {
    return;
  }

  // If the user did not register any callback, just exit.
  if (zwave_api_get_callbacks() == NULL) {
    return;
  }

  switch (pData[IDX_CMD]) {
    case FUNC_ID_ZW_APPLICATION_CONTROLLER_UPDATE: {
      uint8_t bStatus;
      if (zwave_api_get_callbacks()->application_controller_update == NULL) {
        break;
      }

      bStatus = pData[IDX_DATA];
      /* */
      if ((bStatus == UPDATE_STATE_NODE_INFO_FOREIGN_HOMEID_RECEIVED)
          || (bStatus
              == UPDATE_STATE_NODE_INFO_SMARTSTART_HOMEID_RECEIVED_LR)) {
        /* ZW->HOST: 0x49 | 0x85 | Source NodeID (1/2 bytes) | brxStatus
                           | NWI HomeID[0] | NWI HomeID[1] | NWI HomeID[2] | NWI HomeID[3]
                           | nodeInfoLen | BasicDeviceType | GenericDeviceType | SpecificDeviceType
                           | nodeInfo[nodeInfoLen]
             */
        zwave_home_id_t nwi_home_id = 0;
        uint8_t length              = 0;
        uint8_t index               = IDX_DATA + 1;

        zwave_node_id_t source_node_id = zwapi_read_node_id(pData, &index);
        index += 1;  // brxStatus field. We do not use

        memcpy(&nwi_home_id, &pData[index], sizeof(nwi_home_id));
        nwi_home_id = zwave_controller_ntohl(nwi_home_id);
        index += sizeof(nwi_home_id);

        length = pData[index++];
        if (length > sizeof(zwapi_command_buffer)) {
          length = sizeof(zwapi_command_buffer);
        }
        memcpy(zwapi_command_buffer, &pData[index], length);

        zwave_api_get_callbacks()->application_controller_update(
          bStatus,
          source_node_id,
          zwapi_command_buffer,
          length,
          nwi_home_id);

      } else if (bStatus == UPDATE_STATE_INCLUDED_NODE_INFO_RECEIVED) {
        /* ZW-HOST: FUNC_ID_ZW_APPLICATION_UPDATE | UPDATE_STATE_INCLUDED_NODE_INFO_RECEIVED
                    | Source NodeID (1/2 bytes) | Length? | bINIFrxStatus | NWI HomeID [4]
                  E.g. Incoming Z-Wave API frame (hex): Length=0B Type=00 Cmd=49 86 02 05 14 EA 15 15 88
             */
        zwave_home_id_t nwi_home_id = 0;
        uint8_t index               = IDX_DATA + 1;

        zwave_node_id_t source_node_id = zwapi_read_node_id(pData, &index);
        index += 1;  // Length? field. We do not use
        index += 1;  // bINIFrxStatus field. We do not use

        if (len >= (index + sizeof(nwi_home_id))) {
          memcpy(&nwi_home_id, &pData[index], sizeof(nwi_home_id));
          nwi_home_id = zwave_controller_ntohl(nwi_home_id);
          index += sizeof(nwi_home_id);
        }

        zwave_api_get_callbacks()->application_controller_update(bStatus,
                                                                 source_node_id,
                                                                 NULL,
                                                                 0,
                                                                 nwi_home_id);

      } else {
        /* ZW->HOST: 0x49 | bStatus | srcID | bLen
                          | BasicDeviceType | GenericDeviceType | SpecificDeviceType
                          | nodeInfo
                         */
        uint8_t current_index = IDX_DATA;
        learnNodeInfo.bStatus = pData[current_index++];
        learnNodeInfo.bSource = zwapi_read_node_id(pData, &current_index);
        learnNodeInfo.bLen    = pData[current_index++];
        if (zwave_api_detect_buffer_overflow(learnNodeInfo.bLen)) {
          break;
        }
        // The rest of the data is 3 bytes + the node info content
        for (i = 0; i < learnNodeInfo.bLen + 3; i++) {
          zwapi_command_buffer[i] = pData[current_index + i];
        }
        learnNodeInfo.pCmd = zwapi_command_buffer;
        zwave_api_get_callbacks()->application_controller_update(
          learnNodeInfo.bStatus,
          learnNodeInfo.bSource,
          learnNodeInfo.pCmd,
          learnNodeInfo.bLen,
          0);
      }
    } break;

    case FUNC_ID_APPLICATION_COMMAND_HANDLER: {
      if (zwave_api_get_callbacks()->application_command_handler != NULL) {
        // Parse the frame: ZW->HOST: REQ | 0x04 | rxStatus | sourceNode | cmdLength | pCmd[] | rxRSSIVal
        uint8_t current_index               = IDX_DATA;
        uint8_t rx_status                   = pData[current_index++];
        zwave_node_id_t destination_node_id = 0;
        zwave_node_id_t source_node_id
          = zwapi_read_node_id(pData, &current_index);
        uint8_t command_length = pData[current_index++];
        if (zwave_api_detect_buffer_overflow(command_length)) {
          break;
        }
        for (i = 0; i < command_length; i++) {
          zwapi_command_buffer[i] = pData[current_index++];
        }
        int8_t rssi_value = RSSI_NOT_AVAILABLE;
        if (len > current_index) {
          rssi_value = pData[current_index++];
        }
        zwave_api_get_callbacks()->application_command_handler(
          rx_status,
          destination_node_id,
          source_node_id,
          zwapi_command_buffer,
          command_length,
          rssi_value);
      }
    } break;

    case FUNC_ID_PROMISCUOUS_APPLICATION_COMMAND_HANDLER: {
      if (zwave_api_get_callbacks()->application_command_handler) {
        // Parse the frame: ZW->HOST: REQ | 0xD1 | rxStatus | sourceNode | cmdLength | pCmd[] | destNode | multiNodeMaskLen [| multiNodeMask[multiNodeMaskLen]] | rxRSSIVal
        uint8_t current_index  = IDX_DATA;
        uint8_t rx_status      = pData[current_index++];
        uint8_t source_node_id = pData[current_index++];
        uint8_t command_length = pData[current_index++];
        if (zwave_api_detect_buffer_overflow(command_length)) {
          break;
        }
        for (i = 0; i < command_length; i++) {
          zwapi_command_buffer[i] = pData[current_index++];
        }
        uint8_t destination_node_id = pData[current_index++];
        current_index += (pData[current_index] + 1);  // Skip the node_mask
        int8_t rssi_value = RSSI_NOT_AVAILABLE;
        if (len > current_index) {
          rssi_value = pData[current_index++];
        }
        zwave_api_get_callbacks()->application_command_handler(
          rx_status,
          destination_node_id,
          source_node_id,
          zwapi_command_buffer,
          command_length,
          rssi_value);
      }
    } break;

    case FUNC_ID_APPLICATION_COMMAND_HANDLER_BRIDGE: {
      if (zwave_api_get_callbacks()->application_command_handler_bridge) {
        // Parse the frame: ZW->HOST: REQ | 0xA8 | rxStatus | destNodeID | srcNodeID | cmdLength | pCmd[ ] | multiDestsOffset_NodeMaskLen | multiDestsNodeMask | rxRSSIVal
        uint8_t current_index = IDX_DATA;
        uint8_t rx_status     = pData[current_index++];
        zwave_node_id_t destination_node_id
          = zwapi_read_node_id(pData, &current_index);
        zwave_node_id_t source_node_id
          = zwapi_read_node_id(pData, &current_index);
        uint8_t command_length = pData[current_index++];
        if (zwave_api_detect_buffer_overflow(command_length)) {
          break;
        }
        for (i = 0; i < command_length; i++) {
          zwapi_command_buffer[i] = pData[current_index++];
        }
        current_index += (pData[current_index] + 1);  // Skip the node_mask
        // Did we get an RSSI value ?
        int8_t rssi_value = RSSI_NOT_AVAILABLE;
        if (len > current_index) {
          rssi_value = pData[current_index++];
        }
        zwave_api_get_callbacks()->application_command_handler_bridge(
          rx_status,
          destination_node_id,
          source_node_id,
          zwapi_command_buffer,
          command_length,
          rssi_value);
      }
    } break;

      // The rest are callback functions

    case FUNC_ID_ZW_SEND_NODE_INFORMATION:
      if (zwapi_send_node_information_callback != NULL) {
        zwapi_send_node_information_callback(pData[IDX_DATA + 1]);
      }
      break;

    case FUNC_ID_ZW_SEND_DATA_BRIDGE:
    case FUNC_ID_ZW_SEND_DATA:
    case FUNC_ID_SEND_NOP: {
      zwapi_tx_report_t txStatusReport = {};
      fill_default_tx_report_values(&txStatusReport);
      uint8_t *p       = &pData[IDX_DATA + 1];
      uint8_t txStatus = *p++;

      if (pData[IDX_CMD] == FUNC_ID_ZW_SEND_DATA) {
        f                        = zwapi_send_data_callback;
        zwapi_send_data_callback = 0;
      } else if (pData[IDX_CMD] == FUNC_ID_SEND_NOP) {
        f                       = zwapi_send_nop_callback;
        zwapi_send_nop_callback = 0;
      } else {
        f                               = zwapi_send_data_bridge_callback;
        zwapi_send_data_bridge_callback = 0;
      }

      if (len >= 24) {
        txStatusReport.transmit_ticks = (p[0] << 8) | (p[1] << 0);
        p += 2;
        txStatusReport.number_of_repeaters = *p++;
        txStatusReport.ack_rssi            = *p++;

        // Byte 5..8, RSSI values for each repeater.
        txStatusReport.rssi_values.incoming[0] = *p++;
        txStatusReport.rssi_values.incoming[1] = *p++;
        txStatusReport.rssi_values.incoming[2] = *p++;
        txStatusReport.rssi_values.incoming[3] = *p++;
        txStatusReport.ack_channel_number      = *p++;
        txStatusReport.tx_channel_number       = *p++;
        txStatusReport.route_scheme_state      = *p++;

        // Byte 12..15, repeaters NodeIDs, encoded 8 bit no matter what.
        txStatusReport.last_route_repeaters[0] = *p++;
        txStatusReport.last_route_repeaters[1] = *p++;
        txStatusReport.last_route_repeaters[2] = *p++;
        txStatusReport.last_route_repeaters[3] = *p++;

        // Byte 16, beam and last route speed
        txStatusReport.beam_1000ms      = (*p) | (1 << 6);
        txStatusReport.beam_250ms       = (*p) | (1 << 5);
        txStatusReport.last_route_speed = (*p++) & 0x7;

        txStatusReport.routing_attempts      = *p++;
        txStatusReport.last_failed_link.from = *p++;
        txStatusReport.last_failed_link.to   = *p++;

        if (len >= 25) {
          txStatusReport.tx_power = *p++;
        }

        if (len >= 26) {
          txStatusReport.measured_noise_floor = *p++;
        }

        if (len >= 27) {
          txStatusReport.destination_ack_mpdu_tx_power = *p++;
        }

        if (len >= 28) {
          txStatusReport.destination_ack_mpdu_measured_rssi = *p++;
        }

        if (len >= 29) {
          txStatusReport.destination_ack_mpdu_measured_noise_floor = *p++;
        }

        if (f) {
          f(txStatus, &txStatusReport);
        }
      } else {
        if (f) {
          f(txStatus, NULL);
        }
      }
    } break;

    case FUNC_ID_ZW_SEND_TEST_FRAME: {
      if (zwapi_send_test_frame_callback != NULL) {
        f2                             = zwapi_send_test_frame_callback;
        zwapi_send_test_frame_callback = 0;
        f2(pData[IDX_DATA + 1]);
      }
    } break;

    case FUNC_ID_ZW_SEND_DATA_MULTI_BRIDGE:
      if (zwapi_send_data_multi_bridge_callback != NULL) {
        /* (pData + IDX_DATA)[] contains { funcId, txStatus }. Only txStatus is used */
        zwapi_send_data_multi_bridge_callback(pData[IDX_DATA + 1]);
      }
      break;
    case FUNC_ID_ZW_SEND_DATA_MULTI:
      // ZW->HOST: REQ | 0x14 | funcID | txStatus
      if (zwapi_send_data_multi_callback != NULL) {
        zwapi_send_data_multi_callback(pData[IDX_DATA + 1]);
      }
      break;

    case FUNC_ID_MEMORY_PUT_BUFFER:
      if (zwapi_memory_put_buffer_callback != NULL) {
        zwapi_memory_put_buffer_callback();
      }
      break;

    case FUNC_ID_ZW_SET_DEFAULT:
      if (zwapi_set_default_callback != NULL) {
        zwapi_set_default_callback();
      }
      break;

    /// FIXME: We should really not treat those functions as a bulk and
    /// assume they return the same payload.
    case FUNC_ID_ZW_CONTROLLER_CHANGE:
    case FUNC_ID_ZW_CREATE_NEW_PRIMARY:
    case FUNC_ID_ZW_REMOVE_NODE_FROM_NETWORK:
    case FUNC_ID_ZW_ADD_NODE_TO_NETWORK:
      switch (pData[IDX_CMD]) {
        case FUNC_ID_ZW_CONTROLLER_CHANGE:
          funcLearnInfo = zwapi_transfer_primary_ctrl_role_callback;
          break;
        case FUNC_ID_ZW_CREATE_NEW_PRIMARY:
          funcLearnInfo = zwapi_create_new_primarly_ctrl_callback;
          break;
        case FUNC_ID_ZW_REMOVE_NODE_FROM_NETWORK:
          funcLearnInfo = zwapi_remove_node_from_network_callback;
          break;
        case FUNC_ID_ZW_ADD_NODE_TO_NETWORK:
          funcLearnInfo = zwapi_add_node_to_network_callback;
          break;
      }

      if (funcLearnInfo != NULL) {
        memset(&learnNodeInfo, 0, sizeof(learnNodeInfo));

        if (len > IDX_DATA + 1) {
          learnNodeInfo.bStatus = pData[IDX_DATA + 1];
        }
        uint8_t index = IDX_DATA + 2;
        if (len > index) {
          learnNodeInfo.bSource = zwapi_read_node_id(pData, &index);
        }

        if (len > index) {
          learnNodeInfo.bLen = pData[index++];
          learnNodeInfo.pCmd = zwapi_command_buffer;
          if (zwave_api_detect_buffer_overflow(learnNodeInfo.bLen)) {
            break;
          }
          if (learnNodeInfo.bLen > (len - index)) {
            sl_log_warning(
              LOG_TAG,
              "Z-Wave API sent us a length field with a value "
              "higher than the total length of the frame. Clipping");
            learnNodeInfo.bLen = len - index;
          }
          memcpy(zwapi_command_buffer, &pData[index], learnNodeInfo.bLen);
        }

        funcLearnInfo(&learnNodeInfo);
      }
      break;

    case FUNC_ID_ZW_REPLICATION_SEND_DATA:
      if (zwapi_send_controller_replication_callback != NULL) {
        zwapi_send_controller_replication_callback(pData[IDX_DATA + 1]);
      }
      break;

    case FUNC_ID_ZW_ASSIGN_RETURN_ROUTE:
      if (zwapi_assign_return_route_callback != NULL) {
        zwapi_assign_return_route_callback(pData[IDX_DATA + 1]);
      }
      break;

    case FUNC_ID_ZW_DELETE_RETURN_ROUTE:
      if (zwapi_delete_return_route_callback != NULL) {
        zwapi_delete_return_route_callback(pData[IDX_DATA + 1]);
      }
      break;

    case FUNC_ID_ZW_ASSIGN_SUC_RETURN_ROUTE:
      if (zwapi_assign_suc_return_route_callback != NULL) {
        zwapi_assign_suc_return_route_callback(pData[IDX_DATA + 1]);
      }
      break;

    case FUNC_ID_ZW_DELETE_SUC_RETURN_ROUTE:
      if (zwapi_delete_suc_return_route_callback != NULL) {
        zwapi_delete_suc_return_route_callback(pData[IDX_DATA + 1]);
      }
      break;

    case FUNC_ID_ZW_SET_LEARN_MODE:
      if (zwapi_set_learn_mode_callback != NULL) {
        int bytes_to_read = IDX_DATA + 3;
        if (NODEID_16BITS == zwapi_get_node_id_basetype()) {
          bytes_to_read++;
        }
        uint8_t current_index = IDX_DATA + 1;
        if (len > bytes_to_read) {
          learnNodeInfo.bStatus = pData[current_index++];
          learnNodeInfo.bSource = zwapi_read_node_id(pData, &current_index);
          if (zwave_api_detect_buffer_overflow(pData[current_index])) {
            break;
          }
        }

        if (len > (current_index + pData[current_index])) {
          for (i = 0; i < pData[current_index]; i++) {
            zwapi_command_buffer[i] = pData[current_index + 1 + i];
          }
        }
        learnNodeInfo.pCmd = zwapi_command_buffer;
        learnNodeInfo.bLen = pData[current_index];
        zwapi_set_learn_mode_callback(&learnNodeInfo);
      }
      break;

    case FUNC_ID_ZW_SET_VIRTUAL_NODE_TO_LEARN_MODE:
      if (zwapi_set_virtual_node_to_learn_mode_callback != NULL) {
        uint8_t index                    = IDX_DATA + 2;
        zwave_node_id_t original_node_id = zwapi_read_node_id(pData, &index);
        zwave_node_id_t new_node_id      = zwapi_read_node_id(pData, &index);
        zwapi_set_virtual_node_to_learn_mode_callback(pData[IDX_DATA + 1],
                                                      original_node_id,
                                                      new_node_id);
      }
      break;

    case FUNC_ID_ZW_SET_SUC_NODE_ID:
      if (zwapi_set_suc_node_id_callback != NULL) {
        zwapi_set_suc_node_id_callback(pData[IDX_DATA + 1]);
      }
      break;

    case FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE:
      if (zwapi_request_neighbor_update_callback != NULL) {
        zwapi_request_neighbor_update_callback(pData[IDX_DATA + 1]);
      }
      break;

    case FUNC_ID_ZW_REQUEST_NETWORK_UPDATE:
      if (zwapi_request_network_update_callback != NULL) {
        zwapi_request_network_update_callback(pData[IDX_DATA + 1]);
      }
      break;

    case FUNC_ID_ZW_REMOVE_FAILED_NODE_ID:
      if (zwapi_remove_failed_node_callback != NULL) {
        zwapi_remove_failed_node_callback(pData[IDX_DATA + 1]);
      }
      break;

    case FUNC_ID_ZW_REPLACE_FAILED_NODE:
      if (zwapi_replace_failed_node_callback != NULL) {
        zwapi_replace_failed_node_callback(pData[IDX_DATA + 1]);
      }
      break;

    case FUNC_ID_SERIAL_API_STARTED:
      /* ZW->HOST: bWakeupReason | bWatchdogStarted | deviceOptionMask | */
      /*           node_type.generic | node_type.specific | cmdClassLength | cmdClass[] */
      zwapi_set_awaiting_zwave_api_started(false);
      if (zwave_api_get_callbacks()->zwapi_started != NULL) {
        uint8_t data_length
          = len - IDX_DATA - 1;  //-1 to strip off the checksum field
        for (i = 0; i < data_length; i++) {
          zwapi_command_buffer[i] = pData[IDX_DATA + i];
        }
        zwave_api_get_callbacks()->zwapi_started(zwapi_command_buffer,
                                                 data_length);
      }
      break;

    default:
      sl_log_warning(LOG_TAG,
                     "Unknown Z-Wave API FUNC_ID: 0x%02x\n",
                     pData[IDX_CMD]);
  }
}
