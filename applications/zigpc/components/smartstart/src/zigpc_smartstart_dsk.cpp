/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

// Shared component includes
#include <sl_log.h>
#include <smartstart_management.hpp>

// ZigPC includes
#include <zigpc_gateway.h>

// Component includes
#include "zigpc_smartstart_int.hpp"


sl_status_t
  zigpc_smartstart_dsk_parse_install_code(const std::string &dsk,
                                          zigpc_common::dsk_install_code_t &dsk_content)
{
  sl_status_t status = SL_STATUS_OK;
  status = zigpc_common::deserialize_dsk_install_code(dsk, dsk_content);

  // Verify install code format
  if (status == SL_STATUS_OK) {
    bool is_valid
      = zigpc_gateway_install_code_is_valid(dsk_content.install_code,
                                            dsk_content.install_code_length);
    if (is_valid == false) {
      status = SL_STATUS_INVALID_SIGNATURE;
    }
  }

  return status;
}
