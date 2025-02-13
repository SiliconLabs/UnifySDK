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
#include "eed_attribute_mapper.h"

// Other EED Components
#include "eed_attribute_store_attribute_types.h"

// Unify component includes
#include "attribute_mapper.h"

sl_status_t eed_attribute_mapper_init()
{
  attribute_mapper_set_endpoint_id_attribute_type(ATTRIBUTE_ENDPOINT_ID);
  return attribute_mapper_init();
}
