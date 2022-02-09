/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stddef.h>

// Unify includes
#include <datastore_fixt.h>

// ZigPC includes
#include <zigpc_config.h>

// Component includes
#include "zigpc_uic_datastore_fixt.h"

sl_status_t zigpc_uic_datastore_fixt_setup(void)
{
  const zigpc_config_t *config = zigpc_get_config();

  if (config == NULL) {
    return SL_STATUS_FAIL;
  }

  return datastore_fixt_setup_and_handle_version(
    config->datastore_file,
    ZIGPC_CURRENT_DATASTORE_VERSION);
}
