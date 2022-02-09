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
// Includes from this component
#include "zpc_datastore_fixt.h"

// Unify component includes
#include "datastore_fixt.h"
#include "zpc_config.h"

sl_status_t zpc_datastore_fixt_setup()
{
  return datastore_fixt_setup_and_handle_version(
    zpc_get_config()->datastore_file,
    ZPC_DATASTORE_VERSION);
}
