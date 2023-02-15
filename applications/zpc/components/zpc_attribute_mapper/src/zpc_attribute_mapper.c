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
#include "zpc_attribute_mapper.h"

// ZPC Components
#include "attribute_store_defined_attribute_types.h"

// Unify component includes
#include "attribute_mapper.h"

sl_status_t zpc_attribute_mapper_init()
{
  attribute_mapper_set_endpoint_id_attribute_type(ATTRIBUTE_ENDPOINT_ID);
  return attribute_mapper_init();
}
