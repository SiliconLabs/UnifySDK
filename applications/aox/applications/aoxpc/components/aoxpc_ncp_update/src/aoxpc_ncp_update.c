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
#include <stdbool.h>
#include <unistd.h>

#include "aoxpc_config.h"
#include "uart_dfu.h"
#include "sl_log.h"
#include "config.h"
#include "sl_bt_api.h"
#include "sl_bt_ncp_host.h"
#include "aoxpc_ncp_update_fixt.h"

#define LOG_TAG "aoxpc_ncp_update"

static void update_progress(size_t uploaded_size, size_t total_size)
{
  uint32_t percentage = 100 * uploaded_size / total_size;
  printf("\r%zu/%zu (%d%%)", uploaded_size, total_size, percentage);
}

static sl_status_t system_reset(uint8_t boot_mode) {
  sl_status_t sc;
  sl_bt_msg_t evt;
  uint32_t expected_event, received_event;

  switch (boot_mode) {
    case sl_bt_system_boot_mode_normal:
      expected_event = sl_bt_evt_system_boot_id;
      sl_bt_system_reset(boot_mode);
      break;
    case sl_bt_system_boot_mode_uart_dfu:
      expected_event = sl_bt_evt_dfu_boot_id;
      sl_bt_user_reset_to_dfu();
      break;
    default:
      // Unsupported boot mode
      return SL_STATUS_INVALID_PARAMETER;
      break;
  }

  sc = sl_bt_wait_event(&evt);

  if (sc != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Wait event failed, 0x%04x", sc);
    return sc;
  }

  received_event = SL_BT_MSG_ID(evt.header);

  switch (received_event) {
    case sl_bt_evt_system_boot_id:
      sl_log_info(LOG_TAG, "Bluetooth stack booted: v%d.%d.%d-b%d",
                  evt.data.evt_system_boot.major,
                  evt.data.evt_system_boot.minor,
                  evt.data.evt_system_boot.patch,
                  evt.data.evt_system_boot.build);
      break;
    case sl_bt_evt_dfu_boot_id:
      sl_log_info(LOG_TAG, "DFU booted: v0x%08x", evt.data.evt_dfu_boot.version);
      break;
    default:
      sl_log_error(LOG_TAG, "Unexpected event: 0x%08x", received_event);
      break;
  }

  if (received_event != expected_event) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t aoxpc_ncp_update_fixt_setup()
{
  const char *ncp_update_filename;
  config_status_t cfg_status = config_get_as_string(CONFIG_KEY_AOXPC_NCP_UPDATE, &ncp_update_filename);
  bool do_ncp_update = (cfg_status == CONFIG_STATUS_OK) && (ncp_update_filename[0] != '\0');
  bool ncp_version = config_has_flag(CONFIG_KEY_AOXPC_NCP_VERSION) == CONFIG_STATUS_OK;

  if (do_ncp_update || ncp_version) {
    // Just print the firmware version
    (void)system_reset(sl_bt_system_boot_mode_normal);

    if (do_ncp_update) {
      if (system_reset(sl_bt_system_boot_mode_uart_dfu) != SL_STATUS_OK) {
        return SL_STATUS_FAIL;
      }
      sl_log_info(LOG_TAG, "Pressing Crtl+C aborts the update process.");
      sl_log_info(LOG_TAG, "WARNING! If the update process is aborted, the device will"
                            " stay in bootloader mode.");

      sl_status_t sc = uart_dfu_run(ncp_update_filename, update_progress);
      printf("\n");

      if (sc == SL_STATUS_OK) {
        sl_log_info(LOG_TAG, "DFU finished successfully. Resetting the device.");
        if (system_reset(sl_bt_system_boot_mode_normal) != SL_STATUS_OK) {
          return SL_STATUS_FAIL;
        }
      } else {
        sl_log_error(LOG_TAG, "DFU failed.");
        // Print readable error messages for the most common error codes.
        switch (sc) {
          case SL_STATUS_IO:
            sl_log_error(LOG_TAG, "Please check if the file exists: %s",
                          ncp_update_filename);
            break;
          case SL_STATUS_SECURITY_IMAGE_CHECKSUM_ERROR:
            sl_log_error(LOG_TAG, "Please check if the file has valid GBL format: %s",
                          ncp_update_filename);
            break;
          case SL_STATUS_ABORT:
            sl_log_error(LOG_TAG, "Aborted by user.");
            break;
        }
        return SL_STATUS_FAIL;
      }
    }
    return SL_STATUS_ABORT;
  }
  return SL_STATUS_OK;
}
