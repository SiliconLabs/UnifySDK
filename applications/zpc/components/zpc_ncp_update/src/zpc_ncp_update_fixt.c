/******************************************************************************
  * # License
  * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
  ******************************************************************************
  * The licensor of this software is Silicon Laboratories Inc. Your use of this
  * software is governed by the terms of Silicon Labs Master Software License
  * Agreement (MSLA) available at
  * www.silabs.com/about-us/legal/master-software-license-agreement. This
  * software is distributed to you in Source Code format and is governed by the
  * sections of the MSLA applicable to Source Code.
  *
  *****************************************************************************/
#include "zpc_ncp_update_fixt.h"
#include "uic_gbl_interface.h"

#include "zpc_config.h"
#include "zwapi_init.h"
#include <stdlib.h>
#include <sl_log.h>
#include <string.h>

#define LOG_TAG "zpc_ncp_update"

static void update_progress(int offset, int size)
{
  sl_log_info(LOG_TAG, "Transferred %i of %i kb\r", offset / 1000, size / 1000);
}

sl_status_t zpc_ncp_update_fixt_setup()
{
  int serial_fd;

  // Get the Z-Wave module details
  zwapi_chip_data_t chip_data                           = {0};
  zwapi_protocol_version_information_t protocol_version = {0};
  bool do_ncp_update = strlen(zpc_get_config()->ncp_update_filename) > 0;

  if ((zpc_get_config()->ncp_version) || do_ncp_update) {
    if (SL_STATUS_OK
        != zwapi_init(zpc_get_config()->serial_port, &serial_fd, NULL)) {
      sl_log_error(LOG_TAG,
                   "Unable to contact serial api module on %s",
                   zpc_get_config()->serial_port);
      return SL_STATUS_FAIL;
    }

    zwapi_get_chip_data(&chip_data);

    sl_log_info(LOG_TAG,
                "chip_hardware_type     :    %2d",
                chip_data.chip_type);
    sl_log_info(LOG_TAG,
                "chip_hardware_revision :    %2d",
                chip_data.chip_revision);

    if (SL_STATUS_OK == zwapi_get_protocol_version(&protocol_version)) {
      sl_log_info(LOG_TAG,
                  "chip_serial_api_version: %d.%d.%d",
                  protocol_version.major_version,
                  protocol_version.minor_version,
                  protocol_version.revision_version);
      sl_log_info(LOG_TAG,
                  "chip_serial_api_build: %d",
                  protocol_version.build_number);

    } else {
      sl_log_info(LOG_TAG,
                  "chip_serial_api_version: %d.%d",
                  chip_data.version_major,
                  chip_data.version_minor);
    }

    if (do_ncp_update) {
      if (chip_data.chip_type < 7) {
        sl_log_error(
          LOG_TAG,
          "NCP update is only supported for 700 serries chips or newer.");
        zwapi_destroy();
        return SL_STATUS_FAIL;
      }

      if (SL_STATUS_OK != zwapi_enable_auto_program_mode()) {
        sl_log_error(LOG_TAG, "Unable to enable DFU mode.");
        zwapi_destroy();
        return SL_STATUS_FAIL;
      }

      sl_log_info(LOG_TAG, "Re-booting into the bootloader");
      if (SL_STATUS_OK
          != uic_gbl_interface_transfer_image(
            zpc_get_config()->ncp_update_filename,
            serial_fd,
            update_progress)) {
        zwapi_destroy();
        return SL_STATUS_FAIL;
      }
    }

    //Close down the serial api
    zwapi_destroy();

    //Stop the application
    return SL_STATUS_ABORT;
  }

  return SL_STATUS_OK;
}