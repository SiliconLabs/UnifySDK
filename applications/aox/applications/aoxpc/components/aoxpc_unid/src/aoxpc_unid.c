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
// Generic includes
#include <string.h>

// Includes from this component
#include "aoxpc_unid.h"

// Includes unify components
#include "sl_log.h"

#define LOG_TAG "aoxpc_unid"

// Private variables
static uint8_t aoxpc_address[ADR_LEN] = {};
static uint8_t aoxpc_address_type     = 0;
static bool aoxpc_address_set         = false;

///////////////////////////////////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t get_aoxpc_unid(aoa_id_t aoxpc_unid)
{
  if (aoxpc_address_set == false) {
    return SL_STATUS_FAIL;
  }

  aoa_address_to_id(aoxpc_address, aoxpc_address_type, aoxpc_unid);
  return SL_STATUS_OK;
}

sl_status_t set_aoxpc_address(const uint8_t *address, uint8_t address_type)
{
  aoxpc_address_set = true;
  memcpy(aoxpc_address, address, sizeof(aoxpc_address));
  aoxpc_address_type = address_type;

  // Print our brand new UNID
  aoa_id_t aoxpc_unid     = {};
  sl_status_t unid_status = get_aoxpc_unid(aoxpc_unid);
  if (unid_status == SL_STATUS_OK) {
    sl_log_info(LOG_TAG, "AoXPC Address configured. New UNID: %s", aoxpc_unid);
  } else {
    sl_log_error(LOG_TAG, "Could not configure the AoXPC Address/UNID");
  }

  return unid_status;
}

bool is_aoxpc_unid(const aoa_id_t unid)
{
  aoa_id_t aoxpc_unid     = {};
  sl_status_t unid_status = get_aoxpc_unid(aoxpc_unid);

  // If we do not have a UNID, consider everything to be not our UNID.
  if (unid_status != SL_STATUS_OK) {
    return false;
  }

  // Check if this is our UNID.
  if (strcmp(unid, aoxpc_unid) == 0) {
    return true;
  }
  return false;
}
