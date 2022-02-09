/* # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

#include "zpc_config.h"
#include "zwave_rx_test.h"

#include <stdio.h>
/*
 * This is a mockfile for the config system. The CMock framework cannot be used
 * here because this is not a unity test case.
 */

static zpc_config_t my_test_configuration = {
  .serial_log_file = "",
  .serial_port = ZWAVE_RX_TEST_SERIAL_PORT,
  .zwave_normal_tx_power_dbm = ZWAVE_RX_TEST_MAXIMUM_POWER_DBM,
  .zwave_measured_0dbm_power = ZWAVE_RX_TEST_MEASURED_0DBM_POWER,
  .zwave_rf_region = ZWAVE_RX_TEST_RF_REGION_STRING
};

const zpc_config_t *zpc_get_config()
{
  return &my_test_configuration;
}
