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
// Includes from this component
#include "aoxpc_datastore_fixt.h"

// Unify component includes
#include "datastore_fixt.h"

// AoXPC Components
#include "aoxpc_config.h"

sl_status_t aoxpc_datastore_fixt_setup()
{
  const char *datastore_file;

  if (CONFIG_STATUS_OK
      != config_get_as_string(CONFIG_KEY_AOXPC_DATASTORE_FILE, &datastore_file)) {
    return SL_STATUS_FAIL;
  }

  return datastore_fixt_setup_and_handle_version(datastore_file,
                                                 AOXPC_DATASTORE_VERSION);
}
