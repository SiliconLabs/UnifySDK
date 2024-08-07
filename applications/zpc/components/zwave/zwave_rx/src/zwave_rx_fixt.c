/*
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

// Generic includes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Includes from other components
#include "sl_status.h"
#include "zpc_config.h"
#include "uic_main_externals.h"
#include "zwave_api.h"

// Includes from this component
#include "zwave_rx.h"
#include "zwave_rx_internals.h"
#include "zwave_rx_process.h"

// In case of unrecognized RF region, we use the default.
#define ZWAVE_RX_FIXT_DEFAULT_RF_REGION ZWAVE_RF_REGION_EU

/**
 * Convert the configuration string representation of the RF region
 * to the matching value in zwave_rf_region_t
 */
static uint8_t zwave_rx_zpc_config_to_rf_region(const char *region_string)
{
  if (strcmp("EU", region_string) == 0) {
    return ZWAVE_RF_REGION_EU;
  } else if (strcmp("EU_LR", region_string) == 0) {
    return ZWAVE_RF_REGION_EU_LR;
  } else if (strcmp("US", region_string) == 0) {
    return ZWAVE_RF_REGION_US;
  } else if (strcmp("US_LR", region_string) == 0) {
    return ZWAVE_RF_REGION_US_LR;
  } else if (strcmp("ANZ", region_string) == 0) {
    return ZWAVE_RF_REGION_ANZ;
  } else if (strcmp("HK", region_string) == 0) {
    return ZWAVE_RF_REGION_HK;
  } else if (strcmp("IN", region_string) == 0) {
    return ZWAVE_RF_REGION_IN;
  } else if (strcmp("IL", region_string) == 0) {
    return ZWAVE_RF_REGION_IL;
  } else if (strcmp("RU", region_string) == 0) {
    return ZWAVE_RF_REGION_RU;
  } else if (strcmp("CN", region_string) == 0) {
    return ZWAVE_RF_REGION_CN;
  } else if (strcmp("JP", region_string) == 0) {
    return ZWAVE_RF_REGION_JP;
  } else if (strcmp("KR", region_string) == 0) {
    return ZWAVE_RF_REGION_KR;
  }

  return ZWAVE_RX_FIXT_DEFAULT_RF_REGION;
}

sl_status_t zwave_rx_fixt_setup(void)
{
  int zpc_zwave_serial_read_fd = 0;
  sl_status_t rx_init_status   = SL_STATUS_FAIL;

  // Enable serial log before init, so we can see initialization frames too
  rx_init_status = zwapi_log_to_file_enable(zpc_get_config()->serial_log_file);

  // Now initialize the serial port
  if (SL_STATUS_OK == rx_init_status) {
    rx_init_status = zwave_rx_init(
      zpc_get_config()->serial_port,
      &zpc_zwave_serial_read_fd,
      zpc_get_config()->zwave_normal_tx_power_dbm,
      zpc_get_config()->zwave_measured_0dbm_power,
      zpc_get_config()->zwave_max_lr_tx_power_dbm,
      zwave_rx_zpc_config_to_rf_region(zpc_get_config()->zwave_rf_region));
  }

  if (SL_STATUS_OK == rx_init_status) {
    rx_init_status = uic_main_ext_register_rfd(zpc_zwave_serial_read_fd,
                                               NULL,
                                               &zwave_rx_process);
    process_start(&zwave_rx_process, NULL);
  }

  return rx_init_status;
}

int zwave_rx_fixt_teardown()
{
  zwave_rx_shutdown();
  sl_status_t res = zwapi_log_to_file_disable();
  return res;
}
