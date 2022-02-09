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
#include <stdio.h>
#include <unistd.h>

// Includes from this component
#include "positioning_id.h"

// Private variables
static aoa_id_t positioning_id         = "";
static bool positioning_id_set         = false;

///////////////////////////////////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////////////////////////////////
aoa_id_t *get_positioning_id(void)
{
  if(false == positioning_id_set ) {
     snprintf(positioning_id, sizeof(positioning_id), "positioning_%i", getpid());
  }
  return &positioning_id;
}

void set_positioning_id(aoa_id_t id)
{
  positioning_id_set = true;
  aoa_id_copy(positioning_id, id);
}