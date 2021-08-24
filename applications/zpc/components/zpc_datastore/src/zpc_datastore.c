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

#include "zpc_datastore_fixt.h"
#include "datastore_fixt.h"
/**
 * @brief Version 1 of datastore database. 
 * Each datastore version identifies the unique schema of the attribute store
 * The version number should be bumped up when new schema is introduced in 
 * datastore which is no more compatible with current or old schemas. And which
 * requires some sort of data or format conversion.
 */
#define DATASTORE_VERSION_V1 1

sl_status_t zpc_datastore_fixt_setup()
{
  return datastore_fixt_setup_and_handle_version(DATASTORE_VERSION_V1); 
}
