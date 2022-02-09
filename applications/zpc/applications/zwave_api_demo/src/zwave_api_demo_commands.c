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

#include "zwave_api_demo.h"
#include <string.h>

extern bool exit_program;

#define LOG_TAG "zwave_api_demo_commands"

// Displays an help message when the program was called with wrong/no arguments
static void print_commands()
{
  sl_log_info(LOG_TAG, "\nThe following commands are available: \n\
  \t get_ids                                Requests the current HomeID and NodeID from the Z-Wave module and prints it \n\
  \t node_list                              Requests a fresh node list from the Z-Wave module and prints it \n\
  \t lr_node_list                           Requests a fresh Long Range node list from the Z-Wave module and prints it \n\
  \t virtual_node_list                      Requests the list of virtual nodes from the Z-Wave module and prints it \n\
  \t failed_node_list                       Requests the failed node list from the Z-Wave module and prints it \n\
  \t get_rf_region                          Asks the module for its current RF region setting\n\
  \t set_rf_region region                   Sets the RF region setting. Note: requires soft reset to be applied\n\
  \t get_maximum_payload_size               Get the maximum payload that the Z-Wave API Module can accept for transmitting Z-Wave frames.\n\
  \t get_lr_maximum_payload_size            Get the maximum payload that the Z-Wave API Module can accept for transmitting Z-Wave Long Range frames.\n\
  \t get_lr_channel                         Get the Z-Wave Long Range radio channel in use.\n\
  \t set_lr_channel                         Set the Z-Wave Long Range radio channel.\n\
  \t set_nodeid_8bits                       Set node ID basetype to 8 bits\n\
  \t set_nodeid_16bits                      Set node ID basetype to 16 bits\n\
  \t start_learn_mode_inclusion             Starts learn mode Inclusion (direct range)\n\
  \t start_nwi_learn_mode                   Starts NWI learn mode (inclusion only)\n\
  \t start_nwe_learn_mode                   Starts NWE learn mode (exclusion only)\n\
  \t stop_learn_mode                        Stops learn mode\n\
  \t start_add_node                         Starts adding a node \n\
  \t stop_add_node                          Stop adding a node \n\
  \t start_remove_node                      Start removing a node \n\
  \t stop_remove_node                       Stop removing a node \n\
  \t remove_failed_node NodeID              Remove a failed node from the network \n\
  \t send_data NodeID [byte1 byte2 ...]     Sends a raw hexadecimal data to NodeID (e.g. send_data 12 20 02 sends a Basic Get to NodeID 12)\n\
  \t send_basic_set_ff NodeID               Sends a Basic Set (0xff) to NodeID\n\
  \t send_basic_set_00 NodeID               Sends a Basic Set (0x00) to NodeID\n\
  \t send_basic_get NodeID                  Sends a Basic Get to NodeID\n\
  \t send_nif NodeID                        Sends our NIF to NodeID\n\
  \t request_nif NodeID                     Sends a Request NIF command to NodeID\n\
  \t set_default                            Leaves current network and sets the Z-Wave module to default\n\
  \t soft_reset                             Performs a soft reset of the Z-Wave module\n\
  \t quit | exit                            Exits the program \n");
}

static sl_status_t get_ids()
{
  zwave_home_id_t my_home_id = 0;
  zwave_node_id_t my_node_id = 0;
  sl_status_t command_status = zwapi_memory_get_ids(&my_home_id, &my_node_id);
  if (command_status != SL_STATUS_OK) {
    sl_log_warning(
      LOG_TAG,
      "Could not tell retrieve the HomeID and NodeID from the Z-Wave module\n");
  }
  sl_log_info(LOG_TAG, "Z-Wave module HomeID: %8X\n", my_home_id);
  sl_log_info(LOG_TAG, "Z-Wave module NodeID: %d\n", my_node_id);

  return command_status;
}

static sl_status_t start_add_node()
{
  sl_log_info(LOG_TAG, "Starting Add Node.\n");
  return zwapi_add_node_to_network((ADD_NODE_ANY | ADD_NODE_OPTION_NORMAL_POWER
                                    | ADD_NODE_OPTION_NETWORK_WIDE),
                                   zwapi_demo_node_add_callback);
}

static sl_status_t start_learn_mode_inclusion()
{
  sl_log_info(LOG_TAG, "Starting learn mode inclusion (direct range).\n");
  return zwapi_set_learn_mode(ZW_SET_LEARN_MODE_DIRECT_RANGE,
                              zwapi_demo_learn_mode_callback);
}

static sl_status_t start_nwi_learn_mode()
{
  sl_log_info(LOG_TAG, "Starting NWI learn mode.\n");
  return zwapi_set_learn_mode(ZW_SET_LEARN_MODE_NWI,
                              zwapi_demo_learn_mode_callback);
}

static sl_status_t start_nwe_learn_mode()
{
  sl_log_info(LOG_TAG, "Starting NWE learn mode.\n");
  return zwapi_set_learn_mode(ZW_SET_LEARN_MODE_NWE,
                              zwapi_demo_learn_mode_callback);
}

static sl_status_t stop_learn_mode()
{
  sl_log_info(LOG_TAG, "Stopping learn mode.\n");
  return zwapi_set_learn_mode(ZW_SET_LEARN_MODE_DISABLE, NULL);
}

static sl_status_t stop_add_node()
{
  sl_log_info(LOG_TAG, "Stopping Add Node.\n");
  return zwapi_add_node_to_network((ADD_NODE_STOP),
                                   zwapi_demo_node_add_callback);
}

static sl_status_t start_remove_node()
{
  sl_log_info(LOG_TAG, "Starting Remove Node.\n");
  return zwapi_remove_node_from_network(
    (REMOVE_NODE_ANY | REMOVE_NODE_OPTION_NETWORK_WIDE),
    zwapi_demo_node_remove_callback);
}

static sl_status_t stop_remove_node()
{
  sl_log_info(LOG_TAG, "Stopping Remove Node.\n");
  return zwapi_remove_node_from_network(REMOVE_NODE_STOP,
                                        zwapi_demo_node_remove_callback);
}

static sl_status_t remove_failed_node()
{
  // Here we expect the destination NodeID to follow
  sl_status_t command_status = SL_STATUS_FAIL;
  char *token                = strtok(NULL, " ");
  if (token == NULL) {
    sl_log_info(LOG_TAG, "Error: you must enter a NodeID with this command\n");
    return command_status;
  }

  zwave_node_id_t node_id = atoi(token);
  if (IS_ZWAVE_NODE_ID_VALID(node_id)) {
    sl_log_info(LOG_TAG, "Removing failed NodeID %d...\n", node_id);
    uint8_t zwave_return_code
      = zwapi_remove_failed_node(node_id,
                                 zwapi_demo_node_remove_failed_callback);

    if (zwave_return_code == ZW_FAILED_NODE_REMOVE_STARTED) {
      command_status = SL_STATUS_OK;
    }
  }
  return command_status;
}

static void get_rf_region()
{
  uint8_t rf_region = zwapi_get_rf_region();
  sl_log_info(LOG_TAG, "Current RF region setting is: %d\n", rf_region);
}

static sl_status_t set_rf_region()
{
  // Here we expect the RF region to follow
  sl_status_t command_status = SL_STATUS_FAIL;
  char *token                = strtok(NULL, " ");
  if (token == NULL) {
    sl_log_error(LOG_TAG, "You must enter an RF region with this command\n");
    return command_status;
  }
  uint8_t rf_region = atoi(token);
  command_status    = zwapi_set_rf_region(rf_region);
  return command_status;
}

static sl_status_t send_data()
{
  // Here we expect the destination NodeID to follow
  sl_status_t command_status = SL_STATUS_FAIL;
  char *token                = strtok(NULL, " ");
  if (token == NULL) {
    sl_log_info(LOG_TAG, "Error: you must enter a NodeID with this command\n");
    return command_status;
  }

  zwave_node_id_t destination_node_id = atoi(token);
  if (IS_ZWAVE_NODE_ID_VALID(destination_node_id)) {
    // Now we need to pick up the hex data
    token = strtok(NULL, " ");
    uint8_t data_buffer[INPUT_BUFFER_SIZE];
    uint8_t data_buffer_length = 0;
    while (token != NULL) {
      data_buffer[data_buffer_length++] = (int8_t)strtol(token, NULL, 16);
      token                             = strtok(NULL, " ");
    }
    sl_log_info(LOG_TAG,
                "Sending raw data to NodeID %d.\n",
                destination_node_id);

    command_status
      = zwapi_send_data(destination_node_id,
                        data_buffer,
                        data_buffer_length,
                        (TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_EXPLORE),
                        zwapi_demo_send_data_callback);
  }
  return command_status;
}

static sl_status_t send_basic_set(uint8_t value)
{
  // Here we expect the destination NodeID to follow
  sl_status_t command_status = SL_STATUS_FAIL;
  char *token                = strtok(NULL, " ");
  if (token == NULL) {
    sl_log_info(LOG_TAG, "Error: you must enter a NodeID with this command\n");
    return command_status;
  }

  zwave_node_id_t destination_node_id = atoi(token);
  if (IS_ZWAVE_NODE_ID_VALID(destination_node_id)) {
    sl_log_info(LOG_TAG,
                "Sending (non-secure) Basic Set %02X to NodeID %d.\n",
                value,
                destination_node_id);

    uint8_t data_buffer[] = {0x20, 0x01, value};
    command_status
      = zwapi_send_data(destination_node_id,
                        data_buffer,
                        sizeof(data_buffer),
                        (TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_EXPLORE),
                        zwapi_demo_send_data_callback);
  }
  return command_status;
}

static sl_status_t send_basic_get()
{
  // Here we expect the destination NodeID to follow
  sl_status_t command_status = SL_STATUS_FAIL;
  char *token                = strtok(NULL, " ");
  if (token == NULL) {
    sl_log_error(LOG_TAG, "You must enter a NodeID with this command\n");
    return command_status;
  }

  zwave_node_id_t destination_node_id = atoi(token);
  if (IS_ZWAVE_NODE_ID_VALID(destination_node_id)) {
    sl_log_info(LOG_TAG,
                "Sending (non-secure) Basic Get to NodeID %d.\n",
                destination_node_id);

    uint8_t data_buffer[] = {0x20, 0x02};
    command_status
      = zwapi_send_data(destination_node_id,
                        data_buffer,
                        sizeof(data_buffer),
                        (TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_EXPLORE),
                        zwapi_demo_send_data_callback);
  }
  return command_status;
}

static sl_status_t send_nif()
{
  // Here we expect the destination NodeID to follow
  sl_status_t command_status = SL_STATUS_FAIL;
  char *token                = strtok(NULL, " ");
  if (token == NULL) {
    sl_log_info(LOG_TAG, "Error: you must enter a NodeID with this command\n");
    return command_status;
  }

  zwave_node_id_t destination_node_id = atoi(token);
  if (IS_ZWAVE_NODE_ID_VALID(destination_node_id)) {
    sl_log_info(LOG_TAG,
                "Sending our NIF to NodeID %d.\n",
                destination_node_id);

    command_status = zwapi_send_node_information(
      destination_node_id,
      (TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_EXPLORE),
      zwapi_demo_send_nif_callback);
  }
  return command_status;
}

static sl_status_t request_nif()
{
  // Here we expect the destination NodeID to follow
  sl_status_t command_status = SL_STATUS_FAIL;
  char *token                = strtok(NULL, " ");
  if (token == NULL) {
    sl_log_info(LOG_TAG, "Error: you must enter a NodeID with this command\n");
    return command_status;
  }

  zwave_node_id_t destination_node_id = atoi(token);
  if (IS_ZWAVE_NODE_ID_VALID(destination_node_id)) {
    sl_log_info(LOG_TAG,
                "Sending a Request NIF to NodeID %d.\n",
                destination_node_id);

    command_status = zwapi_request_node_info(destination_node_id);
  }
  return command_status;
}

static sl_status_t node_list()
{
  zwave_nodemask_t node_list = {0};

  sl_status_t command_status = zwapi_get_node_list(node_list);
  if (command_status == SL_STATUS_OK) {
    char message[MAXIMUM_MESSAGE_SIZE];
    uint16_t index = 0;
    index
      += snprintf(message + index, sizeof(message) - index, "NodeID List: ");
    for (zwave_node_id_t node_id = ZW_MIN_NODE_ID; node_id <= ZW_LR_MAX_NODE_ID;
         node_id++) {
      if (ZW_IS_NODE_IN_MASK(node_id, node_list) == 1) {
        index
          += snprintf(message + index, sizeof(message) - index, "%d ", node_id);
      }
    }
    sl_log_info(LOG_TAG, "%s\n", message);
  }
  return command_status;
}

static sl_status_t failed_node_list()
{
  zwave_nodemask_t node_list = {0};

  sl_status_t command_status = zwapi_get_full_node_list(node_list);
  if (command_status != SL_STATUS_OK) {
    sl_log_info(LOG_TAG, "Failed to retrieve Z-Wave API node list\n");
    return command_status;
  }
  char message[MAXIMUM_MESSAGE_SIZE];
  uint16_t index = 0;
  index += snprintf(message + index,
                    sizeof(message) - index,
                    "Failed NodeID List: ");
  for (zwave_node_id_t node_id = ZW_MIN_NODE_ID; node_id <= ZW_LR_MAX_NODE_ID;
       node_id++) {
    if (ZW_IS_NODE_IN_MASK(node_id, node_list) == 1) {
      if (zwapi_is_node_failed(node_id)) {
        index
          += snprintf(message + index, sizeof(message) - index, "%d ", node_id);
      }
    }
  }
  sl_log_info(LOG_TAG, "%s\n", message);

  return command_status;
}

static sl_status_t virtual_node_list()
{
  zwave_nodemask_t node_list = {0};

  sl_status_t command_status = zwapi_get_virtual_nodes(node_list);
  if (command_status == SL_STATUS_OK) {
    char message[MAXIMUM_MESSAGE_SIZE];
    uint16_t index = 0;
    index += snprintf(message + index,
                      sizeof(message) - index,
                      "Virtual NodeID List: ");
    for (zwave_node_id_t node_id = ZW_MIN_NODE_ID; node_id <= ZW_LR_MAX_NODE_ID;
         node_id++) {
      if (ZW_IS_NODE_IN_MASK(node_id, node_list) == 1) {
        index
          += snprintf(message + index, sizeof(message) - index, "%d ", node_id);
      }
    }
    sl_log_info(LOG_TAG, "%s\n", message);
  }
  return command_status;
}

static sl_status_t soft_reset()
{
  sl_log_info(LOG_TAG, "Soft reset of the Z-Wave module\n");
  return zwapi_soft_reset();
}

static sl_status_t set_default()
{
  sl_log_warning(LOG_TAG, "Resetting the Z-Wave module to default...\n");
  return zwapi_set_default(zwapi_demo_set_default_callback);
}
static sl_status_t get_maximum_payload_size()
{
  uint8_t maximum_size       = 0;
  sl_status_t command_status = zwapi_get_maximum_payload_size(&maximum_size);
  if (command_status == SL_STATUS_OK) {
    sl_log_info(LOG_TAG, "Maximum Z-Wave payload size: %d\n", maximum_size);
  }
  return command_status;
}

static sl_status_t get_lr_maximum_payload_size()
{
  uint8_t maximum_size       = 0;
  sl_status_t command_status = zwapi_get_lr_maximum_payload_size(&maximum_size);
  if (command_status == SL_STATUS_OK) {
    sl_log_info(LOG_TAG,
                "Maximum Z-Wave Long Range payload size: %d\n",
                maximum_size);
  }
  return command_status;
}

static sl_status_t set_node_id_basetype_8bits()
{
  sl_status_t command_status = zwapi_set_node_id_basetype(NODEID_8BITS);
  if (command_status == SL_STATUS_OK) {
    sl_log_info(LOG_TAG, "Setting node ID basetype to 8 bits\n");
  }
  return command_status;
}

static sl_status_t set_node_id_basetype_16bits()
{
  sl_status_t command_status = zwapi_set_node_id_basetype(NODEID_16BITS);
  if (command_status == SL_STATUS_OK) {
    sl_log_info(LOG_TAG, "Setting node ID basetype to 16 bits\n");
  }
  return command_status;
}

static sl_status_t lr_node_list()
{
  uint16_t len               = 0;
  zwave_nodemask_t node_list = {0};
  sl_status_t command_status = zwapi_get_long_range_nodes(&len, node_list);
  if (command_status == SL_STATUS_OK) {
    sl_log_debug(LOG_TAG, "LR node list in bytes: %d\n", len);
    sl_log_debug(LOG_TAG, "LR node list has node: ");
    for (zwave_node_id_t nodeid = ZW_MIN_NODE_ID; nodeid < ZW_LR_MAX_NODE_ID;
         nodeid++) {
      if (ZW_IS_NODE_IN_MASK(nodeid, node_list)) {
        sl_log_debug(LOG_TAG, "%d ", nodeid);
      }
    }
    sl_log_debug(LOG_TAG, "\n");
  }
  return command_status;
}

static sl_status_t get_lr_channel()
{
  uint8_t lr_channel;
  sl_status_t command_status = zwapi_get_long_range_channel(&lr_channel);
  if (command_status == SL_STATUS_OK) {
    sl_log_debug(LOG_TAG, "LR channel: %d\n", lr_channel);
  }
  return command_status;
}

static sl_status_t set_lr_channel()
{
  sl_status_t command_status = SL_STATUS_FAIL;
  char *token                = strtok(NULL, " ");
  if (token == NULL) {
    sl_log_error(LOG_TAG, "You must enter an LR channel with this command\n");
    return command_status;
  }
  uint8_t lr_channel = atoi(token);
  return zwapi_set_long_range_channel(lr_channel);
}

void process_command(char *input_command)
{
  sl_status_t command_status = SL_STATUS_OK;

  if (strcmp(input_command, "get_ids") == 0) {
    command_status = get_ids();

  } else if (strcmp(input_command, "start_add_node") == 0) {
    command_status = start_add_node();

  } else if (strcmp(input_command, "stop_add_node") == 0) {
    command_status = stop_add_node();

  } else if (strcmp(input_command, "start_learn_mode_inclusion") == 0) {
    command_status = start_learn_mode_inclusion();

  } else if (strcmp(input_command, "start_nwi_learn_mode") == 0) {
    command_status = start_nwi_learn_mode();

  } else if (strcmp(input_command, "start_nwe_learn_mode") == 0) {
    command_status = start_nwe_learn_mode();

  } else if (strcmp(input_command, "stop_learn_mode") == 0) {
    command_status = stop_learn_mode();

  } else if (strcmp(input_command, "start_remove_node") == 0) {
    command_status = start_remove_node();

  } else if (strcmp(input_command, "stop_remove_node") == 0) {
    command_status = stop_remove_node();

  } else if (strcmp(input_command, "remove_failed_node") == 0) {
    command_status = remove_failed_node();

  } else if (strcmp(input_command, "get_rf_region") == 0) {
    get_rf_region();

  } else if (strcmp(input_command, "set_rf_region") == 0) {
    command_status = set_rf_region();

  } else if (strcmp(input_command, "get_lr_channel") == 0) {
    command_status = get_lr_channel();

  } else if (strcmp(input_command, "set_lr_channel") == 0) {
    command_status = set_lr_channel();

  } else if (strcmp(input_command, "get_maximum_payload_size") == 0) {
    command_status = get_maximum_payload_size();

  } else if (strcmp(input_command, "get_lr_maximum_payload_size") == 0) {
    command_status = get_lr_maximum_payload_size();

  } else if (strcmp(input_command, "set_nodeid_8bits") == 0) {
    command_status = set_node_id_basetype_8bits();

  } else if (strcmp(input_command, "set_nodeid_16bits") == 0) {
    command_status = set_node_id_basetype_16bits();

  } else if (strcmp(input_command, "send_data") == 0) {
    command_status = send_data();

  } else if (strcmp(input_command, "send_basic_set_ff") == 0) {
    command_status = send_basic_set(0xFF);

  } else if (strcmp(input_command, "send_basic_set_00") == 0) {
    command_status = send_basic_set(0x00);

  } else if (strcmp(input_command, "send_basic_get") == 0) {
    command_status = send_basic_get();

  } else if (strcmp(input_command, "send_nif") == 0) {
    command_status = send_nif();

  } else if (strcmp(input_command, "request_nif") == 0) {
    command_status = request_nif();

  } else if (strcmp(input_command, "node_list") == 0) {
    command_status = node_list();

  } else if (strcmp(input_command, "lr_node_list") == 0) {
    command_status = lr_node_list();

  } else if (strcmp(input_command, "failed_node_list") == 0) {
    command_status = failed_node_list();

  } else if (strcmp(input_command, "virtual_node_list") == 0) {
    command_status = virtual_node_list();

  } else if (strcmp(input_command, "soft_reset") == 0) {
    command_status = soft_reset();

  } else if (strcmp(input_command, "set_default") == 0) {
    command_status = set_default();

  } else if (strcmp(input_command, "exit") == 0
             || strcmp(input_command, "quit") == 0) {
    exit_program = true;
  } else if (strcmp(input_command, "h") == 0
             || strcmp(input_command, "help") == 0
             || strcmp(input_command, "?") == 0) {
    print_commands();
  } else {
    sl_log_warning(LOG_TAG, "Unknown command. Ignoring\n");
  }

  if (command_status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Could not issue last command to Z-Wave module correctly\n");
  }
}
