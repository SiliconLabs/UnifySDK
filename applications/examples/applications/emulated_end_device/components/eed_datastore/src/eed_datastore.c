/*******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
// Includes from this component
#include "eed_datastore_fixt.h"

// Unify component includes
#include "datastore_fixt.h"
#include "eed_config.h"
#include "datastore.h"
#include "sl_log.h"

#define LOG_TAG "eed_datastore_fixt"

sl_status_t eed_datastore_fixt_setup()
{
  sl_status_t status
    = datastore_fixt_setup_and_handle_version(eed_get_config()->datastore_file,
                                              EED_DATASTORE_VERSION);

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Incompatible version of the EED datastore. "
                 "Please create a new file or migrate the existing one.");
  }

  return status;
}
