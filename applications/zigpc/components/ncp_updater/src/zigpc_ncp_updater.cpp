#include <stdlib.h>
#include <fcntl.h>
#include <sys/file.h>

#include "process.h"
#include "sl_log.h"

#include "uic_gbl_interface.h"
#include "zigpc_gateway.h"
#include "zigpc_config.h"
#include "zigpc_ncp_updater.h"
#include "config.h"

static const char LOG_TAG[] = "zigpc_ncp_updater";

sl_status_t zigpc_ncp_updater_setup(void)
{
  const zigpc_config_t *config = zigpc_get_config();

  if (CONFIG_STATUS_OK != config_has_flag("zigpc.ncp_update")) {
    sl_log_warning(LOG_TAG, "Unexpected NCP update");
    return SL_STATUS_OK;
  }

  {
    sl_log_info(LOG_TAG, "Starting NCP firmware update.\
                All operations aside from the \
                update will not run.");

    sl_status_t status
      = zigpc_ncp_updater_run(config->serial_port,
                              config->ncp_update_firmware_path);

    if (SL_STATUS_OK != status) {
      sl_log_warning(LOG_TAG, "Error restarting NCP in \
                    bootload mode");
      return SL_STATUS_FAIL;
    }

    sl_log_info(LOG_TAG, "Completed NCP update"); 
  }

  //Stop the application
  return SL_STATUS_ABORT;
}

int zigpc_ncp_updater_shutdown(void)
{
  return 0;
}

sl_status_t zigpc_ncp_updater_run(const char *serial_port,
                                  const char *firmware_path)
{
  sl_status_t status = zigpc_gateway_bootload_restart();

  if (SL_STATUS_OK != status) {
    sl_log_warning(LOG_TAG, "Unable to restart NCP in bootloader mode");
    return status;
  }

  int file_descriptor = -1;
  file_descriptor     = open(serial_port, O_RDWR | O_NOCTTY | O_NONBLOCK);

  status = uic_gbl_interface_detect_bootloader(file_descriptor);

  if (SL_STATUS_OK != status) {
    sl_log_warning(LOG_TAG, "No bootloader detected on the given serial port");
    return status;
  }

  status = uic_gbl_interface_transfer_image(firmware_path,
                                            file_descriptor,
                                            zigpc_ncp_update_progress);

  return status;
}

void zigpc_ncp_update_progress(int offset, int size)
{
  double decimal = (double)offset / (double)size;
  double percent = decimal * 100;
  sl_log_debug(LOG_TAG, "NCP is %f percent done", percent);
}
