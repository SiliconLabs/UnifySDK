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

/**
 * @file
 * @brief Bluetooth Network Co-Processor test fixture.
 */

#include <stdint.h>
#include "config.h"
#include "ncp_test.h"
#include "ncp_test_fixt.h"

void ncp_test_config_init(void)
{
  config_status_t status;

  status = config_add_int(
    "ncp_test.interval",
    "Time interval between user events in milliseconds (ignored in echo mode)",
    NCP_TEST_INTERVAL_DEFAULT_MS);
  if (status != CONFIG_STATUS_OK) {
    return;
  }

  status = config_add_int(
    "ncp_test.length",
    "Length of the user event messages in bytes",
    NCP_TEST_LENGTH_DEFAULT);
  if (status != CONFIG_STATUS_OK) {
    return;
  }

  status = config_add_int(
    "ncp_test.count",
    "Number of user event messages to wait for before the test terminates",
    NCP_TEST_COUNT_STOP_DEFAULT);
  if (status != CONFIG_STATUS_OK) {
    return;
  }

  status = config_add_bool(
    "ncp_test.echo",
    "Enable echo mode (synchronous transfer with random data)",
    NCP_TEST_ECHO_MODE_DEFAULT);
  if (status != CONFIG_STATUS_OK) {
    return;
  }
}

sl_status_t ncp_test_setup(void)
{
  config_status_t status;
  int val;

  status = config_get_as_int("ncp_test.interval", &val);
  if (status != CONFIG_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  if ((val <= 0) || (val > UINT8_MAX)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  ncp_test_interval = (uint8_t)val;

  status = config_get_as_int("ncp_test.length", &val);
  if (status != CONFIG_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  if ((val <= 0) || (val > UINT8_MAX)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  ncp_test_length = (uint8_t)val;

  status = config_get_as_int("ncp_test.count", &val);
  if (status != CONFIG_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  if (val <= 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  ncp_test_count_stop = val;

  status = config_get_as_bool("ncp_test.echo", &ncp_test_echo_mode);
  if (status != CONFIG_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  ncp_test_init();
  return SL_STATUS_OK;
}
