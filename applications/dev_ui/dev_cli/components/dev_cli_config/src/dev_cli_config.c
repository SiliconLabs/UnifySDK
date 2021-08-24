/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "dev_cli_config.h"
#include "dev_cli_config_fixt.h"
#include "config.h"
#include "sl_log.h"
#include <string.h>

#define DEFAULT_COMMAND_HELP "help"
#define DEFAULT_DSK          ""
#define DEFAULT_INCLUDE      "true"
#define DEFAULT_PROTOCOLUNID ""
#define DEFAULT_UNID         ""

static dev_cli_config_t config;
int dev_cli_config_init()
{
  memset(&config, 0, sizeof(config));

  // These are options which are supported both command line and in config file
  config_status_t status = CONFIG_STATUS_OK;
  status |= config_add_string("dev_cli.command",
                              "command to execute",
                              DEFAULT_COMMAND_HELP);
  status |= config_add_string("dev_cli.dsk",
                              "DSK key to use for commands",
                              DEFAULT_DSK);
  status |= config_add_string("dev_cli.include",
                              "Include property to use for commands",
                              DEFAULT_INCLUDE);
  status |= config_add_string("dev_cli.protocolunid",
                              "Protocol Controller unid to use for commands",
                              DEFAULT_PROTOCOLUNID);
  status |= config_add_string("dev_cli.unid",
                              "UNID to use for commands",
                              DEFAULT_UNID);

  // NOTE: the options mqtt.host and mqtt.port is already configured in
  // components/config/platform/posix/config.cpp

  return status != CONFIG_STATUS_OK;
}

sl_status_t dev_cli_config_fixt_setup()
{
  config_status_t status = config_get_as_string(CONFIG_KEY_MQTT_HOST, &config.mqtt_host);
  status |= config_get_as_int(CONFIG_KEY_MQTT_PORT, &config.mqtt_port);
  status |= config_get_as_string("dev_cli.command", &config.command);
  status |= config_get_as_string("dev_cli.dsk", &config.dsk);
  status |= config_get_as_string("dev_cli.include", &config.include);
  status |= config_get_as_string("dev_cli.protocolunid", &config.protocolunid);
  status |= config_get_as_string("dev_cli.unid", &config.unid);
  return status == CONFIG_STATUS_OK ? SL_STATUS_OK : SL_STATUS_FAIL;
}

const dev_cli_config_t *dev_cli_get_config(void)
{
  return &config;
}
