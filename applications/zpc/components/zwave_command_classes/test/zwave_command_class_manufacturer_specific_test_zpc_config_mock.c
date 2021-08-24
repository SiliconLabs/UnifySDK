/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zpc_config.h"

static zpc_config_t my_test_configuration = {
  .manufacturer_id = 0x1234,
  .product_type = 0x5678,
  .product_id = 0x9ABC,
  .device_id = "DFCE1324576809",
};

const zpc_config_t *zpc_get_config()
{
  return &my_test_configuration;
}
