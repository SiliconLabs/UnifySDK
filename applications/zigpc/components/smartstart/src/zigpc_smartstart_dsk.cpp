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

/**
 * @brief Seperator used in DSKs
 *
 */
static const char DSK_SEP = '-';

sl_status_t
  zigpc_smartstart_dsk_parse_install_code(const std::string &dsk,
                                          zigpc_dsk_install_code_t &dsk_content)
{
  sl_status_t status = SL_STATUS_OK;
  std::istringstream dsk_iss(dsk);
  std::ostringstream eui64_oss;
  int dsk_element;

  /* EUI64 parsing (fixed length) */
  for (uint8_t i = 0U; (status == SL_STATUS_OK) && (i < sizeof(zigbee_eui64_t));
       i++) {
    dsk_iss >> std::noshowbase >> std::hex >> dsk_element;
    if (dsk_iss.fail() || (dsk_element > 0xFF)) {
      sl_log_debug(LOG_TAG, "Unable to parse EUI64 from DSK");
      status = SL_STATUS_FAIL;
    } else {
      eui64_oss << std::uppercase << std::setfill('0') << std::setw(2)
                << std::hex << dsk_element;
      dsk_content.eui64[i] = static_cast<uint8_t>(dsk_element & 0xFF);
      dsk_iss.ignore(1, DSK_SEP);
    }
  }
  if (status == SL_STATUS_OK) {
    dsk_content.install_code_length = 0U;
    /* Install code parsing (variable length) */
    for (uint8_t i = 0U;
         (status == SL_STATUS_OK) && (i < ZIGBEE_INSTALL_CODE_LENGTH);
         i++) {
      dsk_iss >> std::noshowbase >> std::hex >> dsk_element;
      if (dsk_iss.fail() || (dsk_element > 0xFF)) {
        sl_log_debug(LOG_TAG, "Unable to parse Install Code from DSK");
        status = SL_STATUS_FAIL;
      } else {
        dsk_content.install_code[i] = static_cast<uint8_t>(dsk_element & 0xFF);
        if (dsk_iss.eof()) {
          dsk_content.install_code_length = i + 1;
          break;
        }
        dsk_iss.ignore(1, DSK_SEP);
      }
    }
    if (dsk_content.install_code_length == 0U) {
      return SL_STATUS_FAIL;
    }
  }

  // Verify install code format
  if (status == SL_STATUS_OK) {
    bool is_valid
      = zigpc_gateway_install_code_is_valid(dsk_content.install_code,
                                            dsk_content.install_code_length);
    if (is_valid == false) {
      status = SL_STATUS_INVALID_SIGNATURE;
    }
  }

  if (status == SL_STATUS_OK) {
    dsk_content.dsk = dsk;
  }

  return status;
}
