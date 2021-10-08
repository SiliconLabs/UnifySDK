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

#include "zwave_api_demo.h"

#define LOG_TAG "zwapi_api_demo_callbacks"

void zwapi_demo_application_handler(uint8_t rx_status,
                                    zwave_node_id_t destination_node_id,
                                    zwave_node_id_t source_node_id,
                                    const uint8_t *zwave_command,
                                    uint8_t zwave_command_length,
                                    int8_t rssi_value)
{
  // Here the program will just display the incoming payload.
  char message[MAXIMUM_MESSAGE_SIZE];
  uint16_t index = 0;

  index += snprintf(message + index,
                    sizeof(message) - index,
                    "Z-Wave Command received: ");
  index += snprintf(message + index,
                    sizeof(message) - index,
                    "rx_status: %d - ",
                    rx_status);
  index += snprintf(message + index,
                    sizeof(message) - index,
                    "destination NodeID: %d - ",
                    destination_node_id);
  index += snprintf(message + index,
                    sizeof(message) - index,
                    "source NodeID: %d - ",
                    source_node_id);
  index += snprintf(message + index,
                    sizeof(message) - index,
                    "RSSI value: %d - Payload: ",
                    rssi_value);
  for (uint8_t i = 0; i < zwave_command_length; i++) {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "%02X ",
                      zwave_command[i]);
  }
  sl_log_debug(LOG_TAG, "%s\n", message);
}

void zwapi_demo_application_controller_update(uint8_t status,
                                              zwave_node_id_t node_id,
                                              const uint8_t *zwave_nif,
                                              uint8_t zwave_nif_length,
                                              zwave_home_id_t nwi_home_id)
{
  // Here the program will just display the received data
  char message[MAXIMUM_MESSAGE_SIZE];
  uint16_t index = 0;

  index += snprintf(message + index, sizeof(message) - index, "NIF received: ");
  index += snprintf(message + index,
                    sizeof(message) - index,
                    "status: %d - ",
                    status);
  index += snprintf(message + index,
                    sizeof(message) - index,
                    "NodeID: %d - ",
                    node_id);

  index += snprintf(message + index,
                    sizeof(message) - index,
                    "NWI HomeID: %X - ",
                    nwi_home_id);

  index += snprintf(message + index, sizeof(message) - index, "NIF Contents:");

  for (uint8_t i = 0; i < zwave_nif_length; i++) {
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "%02X ",
                      zwave_nif[i]);
  }
  sl_log_debug(LOG_TAG, "%s\n", message);
}

void zwapi_demo_send_nif_callback(uint8_t tx_status)
{
  if (tx_status == TRANSMIT_COMPLETE_OK) {
    sl_log_debug(LOG_TAG, "NIF transmitted succesfully\n");
  } else if (tx_status == TRANSMIT_COMPLETE_NO_ACK) {
    sl_log_info(LOG_TAG, "NIF not Ack'ed, destination might be sleepy!\n");
  } else if (tx_status == TRANSMIT_COMPLETE_FAIL) {
    sl_log_info(LOG_TAG,
                "NIF could not be transmitted, network may be jammed!\n");
  }
}

void zwapi_demo_node_add_callback(LEARN_INFO *new_node_information)
{
  switch (new_node_information->bStatus) {
    case ADD_NODE_STATUS_LEARN_READY:
      sl_log_info(LOG_TAG,
                  "Protocol is now ready to add nodes, listening for NIFs\n");
      break;

    case ADD_NODE_STATUS_NODE_FOUND:
      sl_log_info(LOG_TAG, "Node found, inclusion ongoing...\n");
      break;

    case ADD_NODE_STATUS_ADDING_END_NODE:
    case ADD_NODE_STATUS_ADDING_CONTROLLER:
      sl_log_info(LOG_TAG,
                  "Node Added (NodeID = %d)\n",
                  new_node_information->bSource);
      zwapi_add_node_to_network(ADD_NODE_STOP, zwapi_demo_node_add_callback);
      break;

    case ADD_NODE_STATUS_PROTOCOL_DONE:
      sl_log_info(
        LOG_TAG,
        "Operation completed. You may invoke controller replication\n");
      break;

    case ADD_NODE_STATUS_DONE:
      sl_log_info(LOG_TAG, "Operation completed. Protocol returns to Idle\n");
      zwapi_add_node_to_network(ADD_NODE_STOP, NULL);
      break;

    case ADD_NODE_STATUS_FAILED:
      zwapi_add_node_to_network(ADD_NODE_STOP, NULL);
      sl_log_warning(LOG_TAG, "Inclusion failed.\n");
      break;

    case ADD_NODE_STATUS_NOT_PRIMARY:
      sl_log_error(LOG_TAG,
                   "Z-Wave module is not primary controller. "
                   "Do not call add node again!\n");
      break;
  }
}

void zwapi_demo_node_remove_callback(LEARN_INFO *new_node_information)
{
  switch (new_node_information->bStatus) {
    case REMOVE_NODE_STATUS_LEARN_READY:
      sl_log_info(
        LOG_TAG,
        "Protocol is now ready to remove nodes, listening for NIFs\n");
      break;

    case REMOVE_NODE_STATUS_NODE_FOUND:
      sl_log_info(LOG_TAG,
                  "Node found (NodeID %d), exclusion ongoing...\n",
                  new_node_information->bSource);
      break;

    case REMOVE_NODE_STATUS_REMOVING_END_NODE:
    case REMOVE_NODE_STATUS_REMOVING_CONTROLLER:
      sl_log_info(LOG_TAG, "Node removed.\n");
      zwapi_remove_node_from_network(REMOVE_NODE_STOP,
                                     zwapi_demo_node_remove_callback);
      break;

    case REMOVE_NODE_STATUS_DONE:
      sl_log_info(LOG_TAG, "Operation completed. Protocol returns to Idle\n");
      zwapi_remove_node_from_network(REMOVE_NODE_STOP, NULL);
      break;

    case REMOVE_NODE_STATUS_FAILED:
      zwapi_remove_node_from_network(REMOVE_NODE_STOP, NULL);
      sl_log_warning(LOG_TAG, "Exclusion failed.\n");
      break;

    case ADD_NODE_STATUS_NOT_PRIMARY:
      sl_log_error(LOG_TAG,
                   "Z-Wave module is not primary controller. Do not call "
                   "remove node again!\n");
      break;
  }
}

void zwapi_demo_node_remove_failed_callback(uint8_t tx_status)
{
  if (tx_status == ZW_NODE_OK) {
    sl_log_info(
      LOG_TAG,
      "The node is operational and removed from the failed node list\n");
  } else if (tx_status == ZW_FAILED_NODE_REMOVED) {
    sl_log_info(LOG_TAG, "The failed node was succesfully removed\n");
  } else if (tx_status == ZW_FAILED_NODE_NOT_REMOVED) {
    sl_log_info(LOG_TAG,
                "An error occurred and the node could not be removed\n");
  }
}

void zwapi_demo_send_data_callback(uint8_t tx_status,
                                   zwapi_tx_report_t *tx_status_report)
{
  char message[MAXIMUM_MESSAGE_SIZE];
  uint8_t index = 0;

  switch (tx_status) {
    case TRANSMIT_COMPLETE_OK:
      index += snprintf(message + index,
                        sizeof(message) - index,
                        "Send data successful (");
      index += snprintf(message + index,
                        sizeof(message) - index,
                        "Repeaters : %d - ",
                        tx_status_report->bRepeaters);
      index += snprintf(message + index,
                        sizeof(message) - index,
                        "Route tries : %d)",
                        tx_status_report->bRouteTries);
      break;

    case TRANSMIT_COMPLETE_NO_ACK:
      index += snprintf(message + index,
                        sizeof(message) - index,
                        "Send data not ACK'ed.");
      break;

    case TRANSMIT_COMPLETE_FAIL:
      index += snprintf(message + index,
                        sizeof(message) - index,
                        "Send data not attempted (jammed network?).");
      break;
  }
  sl_log_info(LOG_TAG, "%s\n", message);
}

void zwapi_demo_request_nif_callback(uint8_t tx_status,
                                     zwapi_tx_report_t *tx_status_report)
{
  char message[MAXIMUM_MESSAGE_SIZE];
  uint8_t index = 0;

  switch (tx_status) {
    case TRANSMIT_COMPLETE_OK:
      index += snprintf(message + index,
                        sizeof(message) - index,
                        "Request NIF successful.");
      index += snprintf(message + index,
                        sizeof(message) - index,
                        "Repeaters : %d - ",
                        tx_status_report->bRepeaters);
      index += snprintf(message + index,
                        sizeof(message) - index,
                        "Route tries : %d",
                        tx_status_report->bRouteTries);
      break;

    case TRANSMIT_COMPLETE_NO_ACK:
      index += snprintf(message + index,
                        sizeof(message) - index,
                        "Request NIF not ACK'ed.");
      break;

    case TRANSMIT_COMPLETE_FAIL:
      index += snprintf(message + index,
                        sizeof(message) - index,
                        "Request NIF not attempted (jammed network?).");
      break;
  }
  sl_log_info(LOG_TAG, "%s\n", message);
}

void zwapi_demo_learn_mode_callback(LEARN_INFO *learn_mode_info)
{
  sl_log_info(LOG_TAG, "Learn mode callback: %d\n", learn_mode_info->bStatus);
  if (learn_mode_info->bStatus == LEARN_MODE_STARTED) {
    sl_log_info(LOG_TAG,
                "New operation (inclusion/exclusion/replication) started\n");
  } else if (learn_mode_info->bStatus == LEARN_MODE_DONE) {
    sl_log_info(LOG_TAG, "Learn mode completed successfully. Back to idle.\n");
    zwapi_set_learn_mode(ZW_SET_LEARN_MODE_DISABLE, NULL);
  } else if (learn_mode_info->bStatus == LEARN_MODE_FAILED) {
    sl_log_error(LOG_TAG, "Learn mode failed. Back to idle.\n");
    zwapi_set_learn_mode(ZW_SET_LEARN_MODE_DISABLE, NULL);
  }
}

void zwapi_demo_set_default_callback()
{
  sl_log_info(
    LOG_TAG,
    "Set default completed! You may start using the Z-Wave module again\n");
}

void zwapi_demo_zwave_api_started(const uint8_t *buffer, uint8_t buffer_length)
{
  // After startup or soft reset,
  // ensure that no add/remove/learn mode is active
  zwapi_add_node_to_network(ADD_NODE_STOP, NULL);
  zwapi_remove_node_from_network(REMOVE_NODE_STOP, NULL);
  zwapi_set_learn_mode(ZW_SET_LEARN_MODE_DISABLE, NULL);

  char message[MAXIMUM_MESSAGE_SIZE];
  uint8_t index = 0;

  index += snprintf(message + index,
                    sizeof(message) - index,
                    "Z-Wave API started. Current NIF: ");
  for (uint8_t i = 0; i < buffer_length; i++) {
    index
      += snprintf(message + index, sizeof(message) - index, "%02X ", buffer[i]);
  }
  sl_log_info(LOG_TAG, "%s\n", message);
}
