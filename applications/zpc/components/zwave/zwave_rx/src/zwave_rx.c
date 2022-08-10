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
//Generic includes
#include <stdlib.h>

// Includes from other components
#include "sl_log.h"
#include "zwapi_init.h"
#include "zwapi_protocol_basis.h"

// Includes from this component
#include "zwave_rx.h"
#include "zwave_rx_internals.h"
#include "zwave_rx_zwapi_callbacks.h"

// Setup Log ID
#define LOG_TAG "zwave_rx"

zwapi_callbacks_t zwave_rx_zwapi_callbacks = {NULL};

// Helper function printing all the detailled information about the Z-Wave chip.
static void zwave_rx_print_chip_data(zwapi_chip_data_t chip_data)
{
  sl_log_info(LOG_TAG, "Z-Wave API module type: %d\n", chip_data.chip_type);
  sl_log_info(LOG_TAG,
              "Z-Wave API module revision: %d\n",
              chip_data.chip_revision);
  sl_log_info(LOG_TAG,
              "Z-Wave API module software version: %d.%d\n",
              chip_data.version_major,
              chip_data.version_minor);
  sl_log_info(LOG_TAG,
              "Z-Wave API module Manufacturer ID 0x%04X\n",
              chip_data.manufacturer_id);
  sl_log_info(LOG_TAG,
              "Z-Wave API module Product type 0x%04X\n",
              chip_data.product_type);
  sl_log_info(LOG_TAG,
              "Z-Wave API module Product ID 0x%04X\n",
              chip_data.product_id);
  sl_log_info(LOG_TAG,
              "Z-Wave API module supported setup mask 0x%02X\n",
              chip_data.supported_setup_bitmask);

  if ((chip_data.capability_flag & GET_INIT_DATA_FLAG_END_NODE_API) == 0) {
    sl_log_info(LOG_TAG, "Z-Wave API module provides a Controller API\n");
  }
  if (chip_data.capability_flag & GET_INIT_DATA_FLAG_TIMER_SUPPORT) {
    sl_log_info(LOG_TAG, "Z-Wave API module provides timer functions\n");
  }
  if ((chip_data.capability_flag & GET_INIT_DATA_FLAG_SECONDARY_CTRL) == 0) {
    sl_log_info(LOG_TAG,
                "Z-Wave API module is primary or inclusion controller\n");
  }
  if (chip_data.capability_flag & GET_INIT_DATA_FLAG_IS_SUC) {
    sl_log_info(LOG_TAG, "Z-Wave API module is SIS in the network\n");
  }

  sl_log_info(LOG_TAG,
              "Z-Wave API module library type: %d\n",
              chip_data.library_type);
  sl_log_info(LOG_TAG, "Z-Wave API module RF region %d\n", chip_data.rf_region);
}

// Helper function printing all the detailled version information for the Z-Wave API Module
static void zwave_rx_print_protocol_version(
  zwapi_protocol_version_information_t zwapi_version)
{
  sl_log_info(LOG_TAG, "Z-Wave API protocol type: %d\n", zwapi_version.type);
  sl_log_info(LOG_TAG,
              "Z-Wave API protocol version: %d.%02d.%02d\n",
              zwapi_version.major_version,
              zwapi_version.minor_version,
              zwapi_version.revision_version);
  sl_log_info(LOG_TAG,
              "Z-Wave API protocol build number: %d\n",
              zwapi_version.build_number);

  char git_commit_string[GIT_COMMIT_HASH_SIZE * 2 + 1] = {0};
  uint16_t index                                       = 0;
  for (uint8_t i = 0; i < GIT_COMMIT_HASH_SIZE; i++) {
    index += snprintf(git_commit_string + index,
                      sizeof(git_commit_string) - index,
                      "%x",
                      zwapi_version.git_commit[i]);
  }

  sl_log_info(LOG_TAG,
              "Z-Wave API protocol git commit: %s\n",
              git_commit_string);
}

sl_status_t zwave_rx_init(const char *serial_port,
                          int *serial_port_fd,
                          int8_t normal_tx_power_dbm,
                          int8_t measured_0dBm_power,
                          zwave_controller_region_t region)
{
  // Register the local Z-Wave RX callbacks to the Z-Wave API callbacks
  zwave_rx_zwapi_callbacks.application_command_handler
    = zwave_rx_application_command_handler;
  zwave_rx_zwapi_callbacks.application_controller_update
    = zwave_rx_application_controller_update;
  zwave_rx_zwapi_callbacks.application_command_handler_bridge
    = zwave_rx_application_command_handler_bridge;
  zwave_rx_zwapi_callbacks.zwapi_started = zwave_rx_zwave_api_started;
  zwave_rx_zwapi_callbacks.poll_request  = zwave_rx_poll_request;
  // Initialize our Z-Wave API.
  sl_status_t command_status
    = zwapi_init(serial_port, serial_port_fd, &zwave_rx_zwapi_callbacks);
  if (command_status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Z-Wave module initialization went wrong.\n");
    return command_status;
  }

  // Get the Z-Wave module details
  zwapi_chip_data_t chip_data = {0};
  zwapi_get_chip_data(&chip_data);
  zwave_rx_print_chip_data(chip_data);

  // Get the Z-Wave module version details:
  zwapi_protocol_version_information_t zwapi_version = {0};
  if (SL_STATUS_OK == zwapi_get_protocol_version(&zwapi_version)) {
    zwave_rx_print_protocol_version(zwapi_version);
  }

  // Verify that we have the Z-Wave module capabilities that we need
  if (chip_data.library_type != ZWAVE_LIBRARY_TYPE_CONTROLLER_STATIC
      && chip_data.library_type != ZWAVE_LIBRARY_TYPE_CONTROLLER
      && chip_data.library_type != ZWAVE_LIBRARY_TYPE_CONTROLLER_BRIDGE) {
    sl_log_error(LOG_TAG, "Z-Wave module library is not a controller.\n");
    return SL_STATUS_FAIL;
  }

  // Configure the RF transmit power
  tx_power_level_t zwave_tx_powerlevel
    = {.normal = normal_tx_power_dbm, .measured0dBm = measured_0dBm_power};
  command_status = zwapi_set_tx_power_level(zwave_tx_powerlevel);
  if (command_status != SL_STATUS_OK
      && command_status != SL_STATUS_NOT_SUPPORTED) {
    sl_log_warning(
      LOG_TAG,
      "Z-Wave module power setting could not be applied. Command status: %d.\n",
      command_status);
    sl_log_warning(LOG_TAG, "Running with default power\n");
  }

  // Even if it is the default, we want to ensure that full
  // TX report is returned after sending data.
  command_status = zwapi_set_tx_status_reporting(true);
  if (command_status != SL_STATUS_OK
      && command_status != SL_STATUS_NOT_SUPPORTED) {
    sl_log_warning(LOG_TAG,
                   "Could not configure the Z-Wave module to return full Tx "
                   "report data. Command status %d\n",
                   command_status);
    // Do not return here, it is not critical for using the API.
  }

  // Set the RF Region if the configuration requested it.
  // if the module does not support it, we still try and fail, we should not run
  // if the user config indicates an RF region and the Z-Wave API runs another.
  if (region != zwapi_get_rf_region()) {
    sl_log_info(LOG_TAG, "Setting the Z-Wave RF region to %d\n", region);
    command_status = zwapi_set_rf_region(region);
    if (command_status == SL_STATUS_NOT_SUPPORTED) {
      sl_log_critical(
        LOG_TAG,
        "Z-Wave API does not support setting the RF Region."
        "It runs with RF Region %d."
        "Please verify that this is in accordance to your configuration",
        zwapi_get_rf_region());
    } else if (command_status != SL_STATUS_OK) {
      sl_log_error(
        LOG_TAG,
        "Could not set the Z-Wave RF Region %d. Command status: %d\n",
        region,
        command_status);
      sl_log_warning(LOG_TAG,
                     "Z-Wave API is running with RF Region %d.\n",
                     zwapi_get_rf_region());
      // Stop everything if the desired RF region cannot be applied.
      return SL_STATUS_FAIL;
    } else {
      sl_log_info(LOG_TAG, "Applying soft reset of the Z-Wave Module\n");
      zwapi_soft_reset();

      // Wait for Z-Wave API started
      zwave_rx_wait_for_zwave_api_to_be_ready();
    }
  }

  // Try to set the node ID basetype to 16 bits disregarding the RF region
  command_status = zwapi_set_node_id_basetype(NODEID_16BITS);
  if (command_status != SL_STATUS_OK) {
    sl_log_info(LOG_TAG,
                "Z-Wave Module runs on 8-bits NodeID basetype setting\n");
  } else {
    sl_log_info(LOG_TAG,
                "Z-Wave Module runs on 16-bits NodeID basetype setting\n");
  }

  return SL_STATUS_OK;
}

void zwave_rx_wait_for_zwave_api_to_be_ready()
{
  while (true == zwapi_is_awaiting_zwave_api_started()) {
    zwapi_poll();
  }
}

void zwave_rx_shutdown()
{
  // Close the serial port with the Z-Wave module
  zwapi_destroy();
}
