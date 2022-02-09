/*******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "aoxpc_config.h"
#include "uic_version.h"

// List of default settings
#define DEFAULT_AOXPC_DATASTORE_FILE       UIC_VAR_DIR "/aoxpc.db"
#define DEFAULT_AOXPC_CONFIG_FILE          ""
#define DEFAULT_AOXPC_NCP_SERIAL_PORT      "/dev/ttyACM0"
#define DEFAULT_AOXPC_NCP_SERIAL_BAUD_RATE 115200
#define DEFAULT_AOXPC_NCP_TCP_ADDRESS      ""

sl_status_t aoxpc_config_init()
{
  config_status_t status = CONFIG_STATUS_OK;

  status |= config_add_string(CONFIG_KEY_AOXPC_CONFIG_FILE,
                              "Configuration file containing locator data",
                              DEFAULT_AOXPC_CONFIG_FILE);

  status |= config_add_string(CONFIG_KEY_AOXPC_DATASTORE_FILE,
                              "AoXPC datastore database file",
                              DEFAULT_AOXPC_DATASTORE_FILE);

  status |= config_add_string(CONFIG_KEY_AOXPC_NCP_SERIAL,
                              "Serial port where NCP module is connected",
                              DEFAULT_AOXPC_NCP_SERIAL_PORT);

  status |= config_add_int(CONFIG_KEY_AOXPC_NCP_BAUD_RATE,
                           "Baud rate of the serial connection",
                           DEFAULT_AOXPC_NCP_SERIAL_BAUD_RATE);

  status |= config_add_string(CONFIG_KEY_AOXPC_NCP_TPC,
                              "TCP address of the connection",
                              DEFAULT_AOXPC_NCP_TCP_ADDRESS);

  return status == CONFIG_STATUS_OK ? SL_STATUS_OK : SL_STATUS_FAIL;
}
