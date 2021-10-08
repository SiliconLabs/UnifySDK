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
#include <string.h>

#define LOG_TAG "zwave_api_demo"

bool exit_program = false;

// Displays an help message when the programm was called with wrong/no arguments
static void print_usage(const char *path_name)
{
  const char *basename = strrchr(path_name, '/');
  basename             = (basename) ? (basename + 1) : path_name;

  sl_log_info(LOG_TAG, "Usage: %s -s serial_device\n", basename);
  sl_log_info(LOG_TAG, "\t -s specifies the path for the Z-Wave module.\n");
}

// Parse the arguments
static sl_status_t parse_main_args(int argc, char **argv, char **serial_device)
{
  int opt;
  *serial_device = NULL;

  while ((opt = getopt(argc, argv, "s:")) != -1) {
    switch (opt) {
      case 's':
        *serial_device = optarg;
        break;
      default:
        return SL_STATUS_FAIL;
    }
  }

  if (*serial_device == NULL) {
    return SL_STATUS_FAIL;
  } else {
    sl_log_info(LOG_TAG, "Using serial device %s\n", *serial_device);
    return SL_STATUS_OK;
  }
}

static void zwapi_demo_print_chip_data(zwapi_chip_data_t chip_data)
{
  sl_log_info(LOG_TAG, "Z-Wave module type: %d\n", chip_data.chip_type);
  sl_log_info(LOG_TAG, "Z-Wave module revision: %d\n", chip_data.chip_revision);
  sl_log_info(LOG_TAG,
              "Z-Wave module software version: %d.%d\n",
              chip_data.version_major,
              chip_data.version_minor);
  sl_log_info(LOG_TAG,
              "Z-Wave module Manufacturer ID 0x%04X\n",
              chip_data.manufacturer_id);
  sl_log_info(LOG_TAG,
              "Z-Wave module Product type 0x%04X\n",
              chip_data.product_type);
  sl_log_info(LOG_TAG,
              "Z-Wave module Product ID 0x%04X\n",
              chip_data.product_id);
  sl_log_info(LOG_TAG,
              "Z-Wave module supported setup mask 0x%02X\n",
              chip_data.supported_setup_bitmask);

  if ((chip_data.capability_flag & GET_INIT_DATA_FLAG_END_NODE_API) == 0) {
    sl_log_info(LOG_TAG, "Z-Wave module provides a Controller API\n");
  }
  if (chip_data.capability_flag & GET_INIT_DATA_FLAG_TIMER_SUPPORT) {
    sl_log_info(LOG_TAG, "Z-Wave module provides a Timer functions\n");
  }
  if ((chip_data.capability_flag & GET_INIT_DATA_FLAG_SECONDARY_CTRL) == 0) {
    sl_log_info(LOG_TAG, "Z-Wave module is primary or inclusion controller\n");
  }
  if (chip_data.capability_flag & GET_INIT_DATA_FLAG_IS_SUC) {
    sl_log_info(LOG_TAG, "Z-Wave module is SIS in the network\n");
  }

  sl_log_info(LOG_TAG,
              "Z-Wave module library type: %d\n",
              chip_data.library_type);
  sl_log_info(LOG_TAG, "Z-Wave module RF region %d\n", chip_data.rf_region);
}

int main(int argc, char **argv)
{
  // Initializes the logging feature
  sl_log_info(LOG_TAG, "Welcome !\n");

  // Parse the arguments.
  char *serial_device = NULL;
  if (parse_main_args(argc, argv, &serial_device) != SL_STATUS_OK) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  // Initialize the zwave API
  const zwapi_callbacks_t callbacks = {
    .application_command_handler   = zwapi_demo_application_handler,
    .application_controller_update = zwapi_demo_application_controller_update,
    .application_command_handler_bridge = zwapi_demo_application_handler,
    .zwapi_started                      = zwapi_demo_zwave_api_started};

  sl_status_t init_status = zwapi_init(serial_device, NULL, &callbacks);

  if (init_status != SL_STATUS_OK) {
    sl_log_critical(LOG_TAG,
                    "Z-Wave module initialization went wrong. Exiting\n");
    return EXIT_FAILURE;
  }

  // Get the Z-Wave module details
  zwapi_chip_data_t chip_data = {0};
  zwapi_get_chip_data(&chip_data);
  zwapi_demo_print_chip_data(chip_data);

  // Configure TX reports to be returned after send_data
  zwapi_set_tx_status_reporting(true);

  // Set our Node Information Frame (NIF)
  node_type_t my_node_type = {.basic = 0x01, .generic = 0x02, .specific = 0x03};
  uint8_t my_nif[]         = {0x25, 0x9F, 0x36};
  sl_status_t command_status
    = zwapi_set_application_node_information(APPLICATION_NODEINFO_LISTENING,
                                             my_node_type,
                                             my_nif,
                                             sizeof(my_nif));
  if (command_status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Could not tell the Z-Wave module to setup our NIF\n");
  }

  // Now a user loop where they can decide what to do:
  sl_log_info(LOG_TAG,
              "Starting user input loop. Please enter your command. Type "
              "'help' or '?' for help.\n");
  char input_buffer[INPUT_BUFFER_SIZE];
  uint16_t input_length = 0;

  while (!exit_program) {
    // Receive all the frames from the Z-Wave module first
    while (zwapi_poll()) {
    };

    // Then wait maximum 100ms for user input as we need to "zwapi_poll" regularly
    input_length = read_command_line(input_buffer, INPUT_BUFFER_SIZE, 100);

    if (input_length > 0) {
      char *command = strtok(input_buffer, " ");
      while (command != NULL) {
        process_command(command);
        command = strtok(NULL, " ");
      }
    }
  }

  sl_log_info(LOG_TAG, "Exiting\n");

  return EXIT_SUCCESS;
}
